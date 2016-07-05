/*
 * Reference: http://blog.oasisfeng.com/2006/10/19/full-cjk-unicode-range/
 * Some code come from http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 *
 Copyright (c) 2012-2016 Kuan-Chung Chiu <buganini@gmail.com>

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.
 */

#include <stdlib.h>
#include <errno.h>
#include <bsdconv.h>
#include <opencc/opencc.h>
#include <string.h>

struct my_s {
	struct bsdconv_instance *insU8;
	struct bsdconv_instance *ins8U;
	opencc_t cc;
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
	struct my_s *r=THIS_CODEC(ins)->priv=malloc(sizeof(struct my_s));
	char *ini=NULL;

	while(arg){
		ini=arg->key;
		arg=arg->next;
	}

	if(ini==NULL)
		return EINVAL;

	r->cc=opencc_open(ini);
	if(r->cc==(opencc_t) -1){
		free(r);
		return EOPNOTSUPP;
	}

	r->insU8 = bsdconv_create("PASS:UNICODE:UTF-8");
	r->ins8U = bsdconv_create("UTF-8:PASS");

	return 0;
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	bsdconv_init(r->insU8);
	bsdconv_init(r->ins8U);
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=THIS_CODEC(ins)->priv;
	bsdconv_destroy(r->insU8);
	bsdconv_destroy(r->ins8U);
	opencc_close(r->cc);
	free(r);
}

void cbflush(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;

	r->insU8->output.len = 0;
	r->insU8->output_mode = BSDCONV_AUTOMALLOC;
	r->insU8->flush = 1;
	bsdconv(r->insU8);
	char *utf8 = opencc_convert_utf8(r->cc, r->insU8->output.data, r->insU8->output.len);
	bsdconv_free(r->insU8->output.data);

	bsdconv_init(r->ins8U);
	r->ins8U->input.data = utf8;
	r->ins8U->input.len = strlen(utf8);
	r->ins8U->input.flags = 0;
	r->ins8U->input.next = NULL;
	r->ins8U->output_mode = BSDCONV_HOLD;
	r->ins8U->flush = 1;
	bsdconv(r->ins8U);
	opencc_convert_utf8_free(utf8);

	struct bsdconv_phase *last = LAST_PHASE(r->ins8U);
	this_phase->data_tail->next = last->data_head->next;
	last->data_head->next = NULL;
	last->data_tail = last->data_head;
	while(this_phase->data_tail->next){
		this_phase->data_tail = this_phase->data_tail->next;
	}
	this_phase->state.status=NEXTPHASE;
}

void cbconv(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=THIS_PHASE(ins);
	struct my_s *r=THIS_CODEC(ins)->priv;
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
		r->insU8->input.data = data;
		r->insU8->input.len = this_phase->curr->len;
		r->insU8->input.flags = 0;
		r->insU8->input.next = NULL;
		r->insU8->flush = 0;
		r->insU8->output_mode = BSDCONV_HOLD;
		bsdconv(r->insU8);

		this_phase->state.status=SUBMATCH;
		return;
	}

	cbflush(ins);

	this_phase->data_tail->next=dup_data_rt(ins, this_phase->curr);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;
}
