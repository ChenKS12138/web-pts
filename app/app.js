const koa = require("koa");
const path = require("path");
const koaStatic = require("koa-static");
const koaWebsocket = require("koa-websocket");
const { PtsStream } = require("./util/pts-stream");
const koaNunjucks = require("koa-nunjucks-2");

const app = koaWebsocket(new koa());

app.use(
  koaNunjucks({
    ext: "njk",
    path: path.join(__dirname, "views"),
    nunjucksConfig: {
      trimBlocks: true,
    },
  })
);

app.use(async (ctx, next) => {
  ctx.render("index");
  await next();
});

app.use(koaStatic(path.join(__dirname, "static")));

app.ws.use((ctx) => {
  const ptsStream = new PtsStream();
  ctx.websocket.send(
    Buffer.from("Welcome To Use web-pts\r\n" + new Date() + "\r\n")
  );
  ctx.websocket.on("message", (message) => {
    switch (message[0]) {
      case "0":
        ptsStream.write(message.slice(2));
        break;
      case "1":
        const [_type, rows, cols] = String(message).split(",");
        ptsStream.resize(rows, cols);
        break;
    }
  });
  ptsStream.on("data", (chunk) => {
    ctx.websocket.send(chunk);
  });
  ctx.websocket.on("close", () => {
    ptsStream.destroy();
  });
});

app.listen(3000);
