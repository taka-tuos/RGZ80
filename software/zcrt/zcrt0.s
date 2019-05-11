	; モジュール名とグローバル宣言は一緒です。
	.module	crt0
	.globl	_main

	; リンカへのエリア指定ですが、使うものだけでも構いません。
	; コード(_CODE)、初期化ルーチン(_GSINIT)、データ(_DATA)はsdccで出力するので必須です。
	; _GSFINALは初期化ルーチンから戻るために必須です。_DATAFINALは別に必須ではないのですが、
	; 大域変数の初期化時にデータ領域の終了アドレスが必要なので入れています。
	.area	_CODE
	.area	_GSINIT
	.area	_GSFINAL
	.area	_DATA
	.area	_DATAFINAL

	; コードセグメントの最初の部分です。
	; 0番地から飛んできませんが、リンカの--code-loc指定でここから開始するので問題ありません。
	.area	_CODE
init::
	; グローバル変数が入るデータエリア(_datastartから_dataend-1まで)を0で初期化します。
	ld	hl, #_datastart
	ld	bc, #_dataend
_clear_loop:
	ld	a, h
	sub	b
	jr	nz, _clear_next
	ld	a, l
	sub	c
	jr	z, _clear_exit
_clear_next:
	ld	(hl), #0
	inc	hl
	jr	_clear_loop
_clear_exit:
	; グローバル変数初期化ルーチンを呼び出します。
	call	gsinit
	; メインに飛びます。
	; メインがretするとプログラム自体の呼び出し元に返ります。
	jp	_main
	
	.area	_GSINIT
gsinit::
	
	.area	_GSFINAL
	ret
	
	.area	_DATA
_datastart::
	
	.area	_DATAFINAL
_dataend::
