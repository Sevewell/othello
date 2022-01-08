# Dockerイメージのビルド
`docker build -t sevewell/othello_<function> ./<function>`

# Dockerコンテナの実行
`docker run -p 80:80 --env-file <function>.env sevewell/othello_<function>:latest`