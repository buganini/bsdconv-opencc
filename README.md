#About

bsdconv-opencc is OpenCC inter-conversion module for bsdconv. OpenCC cannot do chunked conversion, while bsdconv can accept chunked arbitrary byte array, segment content with non-chinese characters, then convert each segments with opencc.

bsdconv-opencc 使用 OpenCC 做中間轉換，使用 bsdconv-opencc 的額外好處是 bsdconv-opencc 能夠接受串流輸入，而 OpenCC 只能接受整塊的輸入，使用 bsdconv-opencc 的時候，內部會依據非中文的符號分段送給 OpenCC 處理。

#Example

	echo 测试 | bsdconv utf-8:opencc#s2t.json:utf-8

See https://github.com/BYVoid/OpenCC for more informantion.
