FROM gcc:9
WORKDIR /usr/src/app
COPY ./ .
RUN gcc search.c -lm -o search

FROM node:12
WORKDIR /usr/src/app
COPY --from=0 /usr/src/app .
RUN npm install
ENV TRIAL=1000000
ENV LEARNING_RATE=0.99
ENV TREE=4
EXPOSE 8080

CMD ["npm", "start"]