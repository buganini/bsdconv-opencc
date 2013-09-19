/*
 * Reference: http://blog.oasisfeng.com/2006/10/19/full-cjk-unicode-range/
 * Some code come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 *
 * Copyright (c) 2012 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <errno.h>
#include <bsdconv.h>
#include <opencc/opencc.h>
#include <string.h>

struct ucs4_s {
	size_t c;
	struct ucs4_s *next;
};

struct my_s {
	opencc_t cc;
	struct ucs4_s *qh, *qt;
};

struct range {
	int first;
	int last;
};

static const struct range zhrange[] = {
	{ 0x3100, 0x312F },	//Chinese Bopomofo
	{ 0x3400, 0x4DB5 },	//CJK Unified Ideographs Extension A	;Unicode3.0
	{ 0x4E00, 0x6FFF },	//CJK Unified Ideographs	;Unicode 1.1	;HF
	{ 0x7000, 0x9FA5 },	//CJK Unified Ideographs	;Unicode 1.1	;LF
	{ 0x9FA6, 0x9FBB },	//CJK Unified Ideographs	;Unicode 4.1
	{ 0xF900, 0xFA2D },	//CJK Compatibility Ideographs	;Unicode 1.1
	{ 0xFA30, 0xFA6A },	//CJK Compatibility Ideographs	;Unicode 3.2
	{ 0xFA70, 0xFAD9 },	//CJK Compatibility Ideographs	;Unicode 4.1
	{ 0x20000, 0x2A6D6 },//CJK Unified Ideographs Extension B	;Unicode 3.1
	{ 0x2F800, 0x2FA1D },//CJK Compatibility Supplement	;Unicode 3.1
};

int cbcreate(struct bsdconv_instance *ins, struct bsdconv_hash_entry *arg){
	struct my_s *r=CURRENT_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	r->cc=opencc_open(BSDCONV_OPENCC_CONVERSION);
	if(!r->cc){
		free(r);
		return EOPNOTSUPP;
	}
	r->qh=malloc(sizeof(struct ucs4_s));
	r->qh->next=NULL;
	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	struct ucs4_s *t;
	while(r->qh->next){
		t=r->qh->next->next;
		free(r->qh->next);
		r->qh->next=t;
	}
	r->qt=r->qh;
	r->qh->c=0;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	struct ucs4_s *t;
	opencc_close(r->cc);
	while(r->qh){
		t=r->qh->next;
		free(r->qh);
		r->qh=t;
	}
	free(r);
}

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	struct ucs4_s *t;
	int i,j;
	size_t m=r->qh->c;
	size_t n=r->qh->c * 2;
	size_t on=n;

	ucs4_t ib[m];
	ucs4_t ob[n];
	ucs4_t *ic, *oc;

	ic=ib;
	while(r->qh->next){
		*ic=r->qh->next->c;
		t=r->qh->next->next;
		free(r->qh->next);
		r->qh->next=t;
		ic+=1;
	}
	r->qt=r->qh;
	r->qh->c=0;

	ic=ib;
	while(m){
		oc=ob;
		n=on;
		opencc_convert(r->cc, &ic, &m, &oc, &n);
		for(i=0;i<on-n;++i){
			j=0;
			if(ob[i] & (0xff << 8*3)){
				j=5;
			}else if(ob[i] & (0xff << 8*2)){
				j=4;
			}else if(ob[i] & (0xff << 8*1)){
				j=3;
			}else if(ob[i] & (0xff << 8*0)){
				j=2;
			}else{
				j=1;
			}

			DATA_MALLOC(this_phase->data_tail->next);
			this_phase->data_tail=this_phase->data_tail->next;
			this_phase->data_tail->next=NULL;
			this_phase->data_tail->data=malloc(j);
			this_phase->data_tail->flags|=F_FREE;
			UCP(this_phase->data_tail->data)[0]=0x01;
			this_phase->data_tail->len=j;

			uint32_t ucs=ob[i];
			j-=1;
			while(j){
				UCP(this_phase->data_tail->data)[j]=ucs & 0xff;
				ucs>>=8;
				j-=1;
			}
		}
	}

	this_phase->state.status=NEXTPHASE;
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	unsigned char *data=data=this_phase->curr->data;
	int ucs=0;
	int i;
	int max=sizeof(zhrange) / sizeof(struct range) - 1;
	int min = 0;
	int mid;
	int isChinese=0;

	for(i=1;i<this_phase->curr->len;++i){
		ucs<<=8;
		ucs|=data[i];
	}

	if (ucs < zhrange[0].first || ucs > zhrange[max].last){
		//noop
	}else while (max >= min) {
		mid = (min + max) / 2;
		if (ucs > zhrange[mid].last)
			min = mid + 1;
		else if (ucs < zhrange[mid].first)
			max = mid - 1;
		else{
			isChinese=1;
			break;
		}
	}

	if(isChinese){
		r->qh->c+=1;

		r->qt->next=malloc(sizeof(struct ucs4_s));
		r->qt=r->qt->next;
		r->qt->c=ucs;
		r->qt->next=NULL;
		this_phase->state.status=SUBMATCH;
		return;
	}

	cbflush(ins);

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	*(this_phase->data_tail)=*(this_phase->curr);
	this_phase->curr->flags &= ~F_FREE;
	this_phase->data_tail->next=NULL;
}
