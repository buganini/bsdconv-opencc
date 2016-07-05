#About

bsdconv-opencc is OpenCC inter-conversion module for bsdconv. OpenCC cannot do chunked conversion, while bsdconv can accept chunked arbitrary byte array, segment content with non-chinese characters, then convert each segments with opencc.

#Example

	echo 测试 | bsdconv utf-8:opencc#s2t.json:utf-8

See https://github.com/BYVoid/OpenCC for more informantion.
