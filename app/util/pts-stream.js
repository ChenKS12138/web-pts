const stream = require("stream");
const fs = require("fs");
const ptsAddon = require("./pts-addon");

class PtsStream extends stream.Duplex {
  constructor(options) {
    super(options);
    const { ptsFd, childPid } = ptsAddon.createPts();
    this.ptsFd = ptsFd;
    this.childPid = childPid;
  }
  _destroy() {
    fs.close(this.ptsFd);
    ptsAddon.killPtsChild(this.childPid);
  }
  _read(size) {
    fs.read(
      this.ptsFd,
      {
        length: size,
      },
      (err, bytesRead, buffer) => {
        if (err || bytesRead == 0) {
          this.destroy(err);
          return;
        }
        this.push(Buffer.from(buffer).slice(0, bytesRead));
      }
    );
  }
  _write(chunk, encoding, callback) {
    fs.write(this.ptsFd, chunk, callback);
  }
  resize(rows, cols) {
    ptsAddon.resizePts(this.ptsFd, rows, cols);
  }
}

module.exports = { PtsStream };
