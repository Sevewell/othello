# Overview
オセロAIと対局できるプログラムです。仕組みについては記事をご覧ください。  
https://sevenwell.hatenablog.com/entry/2020/02/11/112707  
インターフェースはPythonで、エンジンはCで実装しています。

# Requirement
CPU x86_64  
Windows

# Installation
`docker build -t othello ./engine`
`docker run -v $PWD/engine:/host --rm othello`

# Usage
`python main.py`
