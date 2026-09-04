# Overview

オセロのAIです。学習させて使います。対戦もできます。

# Mechanism

"確率的モンテカルロ木探索"です。各局面の勝率Pを出力するベータ分布の母数を、シミュレーションで推定していきます。

# Requirement

- Rust
- Python

# Installation

"engine"フォルダのRustパッケージをビルドしてください。

# Settings

- Environment Variables
    - DB_LIMIT_DEPTH：DBに保存する学習ツリーの深さ

# Usage

`python main.py`
