# Overview
オセロAIと対局できるプログラムです。仕組みについては記事をご覧ください。  
https://sevenwell.hatenablog.com/entry/2020/02/11/112707  
インターフェースはPythonで、エンジンはCで実装しています。

# Requirement
CPU x86_64  
Linux kernel5.4.0  
GCC 9.3.0  

# Installation
`gcc search.c -lm -o search`

# Usage
`python main.py $RANDOM`