# Overview
オセロAIと対局できるプログラムです。仕組みについては記事をご覧ください。  
https://sevenwell.hatenablog.com/entry/2020/02/11/112707  
インターフェースはPythonで、エンジンはCで実装しています。

# Requirement
Python3とCコンパイラが必要です。CMakeでの動作のみ確認していますが、GCCなどでも動作するはずです。  
Windows環境を想定していますが、Linuxでも動作すると思います。Windows固有のライブラリなどは使用していないので、互換は容易かと思います。  
外部ライブラリは使用しておりません。

# Installation
venvの使用をお勧めします。  
`python -m venv env`  
`env/Script/activate`  
Cプログラムをコンパイルします。ビルド物は配布していませんすみません。  
`python setup.py install`

# Usage
`python main.py`