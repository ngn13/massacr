FROM node:23.6.1 as build

WORKDIR /app
COPY . /app

RUN npm install && npm run build

FROM oven/bun:1.2 as main

WORKDIR /app

COPY --from=build /app/build ./build
COPY --from=build /app/package.json ./package.json
COPY --from=build /app/package-lock.json ./package-lock.json

RUN bun install
CMD ["bun", "build/index.js"]
