#About

bsdconv-opencc is OpenCC inter-conversion module for bsdconv. OpenCC cannot do chunked conversion, while bsdconv can accept chunked arbitrary byte array, segment content with non-chinese characters, then convert each segments with opencc.

#Example

	echo 测试 | bsdconv utf-8:opencc#zhs2zhtw_vp.ini:utf-8

#Available OpenCC conversion profile

According to my current installed opencc, there are

	mix2zhs
	mix2zht
	zhs2zht
	zhs2zhtw_p
	zhs2zhtw_vp
	zhs2zhtw_v
	zht2zhs
	zht2zhtw_p
	zht2zhtw_vp
	zht2zhtw_v
	zhtw2zhcn_s
	zhtw2zhcn_t
	zhtw2zhs
	zhtw2zht

See http://code.google.com/p/opencc/ for more informantion.
