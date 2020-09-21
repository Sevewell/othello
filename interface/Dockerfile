FROM node:12

WORKDIR /usr/src/app

COPY ./ .

RUN npm install

ENV GOOGLE_APPLICATION_CREDENTIALS="./othello-290003-36ec740065b5.json"

EXPOSE 3000

CMD ["npm", "start"]