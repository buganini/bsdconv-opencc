/*
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

#include <bsdconv.h>
#include <opencc/opencc.h>
#include <string.h>

struct my_s{
	opencc_t cc;
	struct data_s *q;
};

void cbcreate(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv=malloc(sizeof(struct my_s));
//	r->cc=opencc_open(OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD);
	r->cc=opencc_open(OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP);
}

void cbinit(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	r->q=NULL;
}

void cbdestroy(struct bsdconv_instance *ins){
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	opencc_close(r->cc);
	free(r);
}

void callback(struct bsdconv_instance *ins){
	struct bsdconv_phase *this_phase=CURRENT_PHASE(ins);
	struct my_s *r=CURRENT_CODEC(ins)->priv;
	ucs4_t ib[1];
	ucs4_t ob[1];
	ucs4_t *ic=ib, *oc=ob;
	size_t m,n;
	int i;

	ib[0]=0;
	for(i=1;i<this_phase->curr->len;++i){
		ib[0]<<=8;
		ib[0]|=UCP(this_phase->curr->data)[i];
	}

	m=n=1;
	i=opencc_convert(r->cc, &ic, &m, &oc, &n);

	i=0;
	if(ob[0] & (0xff << 8*3)){
		i=5;
	}else if(ob[0] & (0xff << 8*2)){
		i=4;
	}else if(ob[0] & (0xff << 8*1)){
		i=3;
	}else if(ob[0] & (0xff << 8*0)){
		i=2;
	}else{
		i=1;
	}

	DATA_MALLOC(this_phase->data_tail->next);
	this_phase->data_tail=this_phase->data_tail->next;
	this_phase->data_tail->next=NULL;
	this_phase->data_tail->data=malloc(i);
	this_phase->data_tail->flags|=F_FREE;
	UCP(this_phase->data_tail->data)[0]=0x01;
	this_phase->data_tail->len=i;

	uint32_t t=ob[0];
	i-=1;
	while(i){
		UCP(this_phase->data_tail->data)[i]=t & 0xff;
		t>>=8;
		i-=1;
	}

	this_phase->state.status=NEXTPHASE;
}
