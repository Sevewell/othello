# ベースイメージを指定
FROM amd64/debian:stable

# 64ビット版のGCCをインストール
RUN dpkg --add-architecture amd64 \
    && apt-get update \
    && apt-get install -y gcc gcc-mingw-w64-x86-64 \
    && rm -rf /var/lib/apt/lists/*

# 作業ディレクトリを指定
WORKDIR /app

# ローカルのソースコードをコンテナにコピー
COPY . /app

# Windows向けにクロスコンパイル
RUN x86_64-w64-mingw32-gcc -o othello.exe search.c

# コンパイルした実行ファイルをホストにコピー
CMD ["cp", "othello.exe", "/host"]
