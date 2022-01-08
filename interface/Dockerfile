FROM node:12

WORKDIR /usr/src/app

COPY ./ .

RUN npm install

EXPOSE 80
EXPOSE 443

CMD ["npm", "start"]