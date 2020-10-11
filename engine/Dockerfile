FROM gcc:9
WORKDIR /usr/src/app
COPY ./ .
RUN gcc search.c -lm -o search

FROM node:12
WORKDIR /usr/src/app
COPY --from=0 /usr/src/app .
RUN npm install
EXPOSE 8080

CMD ["npm", "start"]