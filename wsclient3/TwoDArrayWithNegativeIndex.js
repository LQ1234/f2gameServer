class TwoDArrayWithNegativeIndex {
  constructor() {

    this.obj = {};
  }
  put(c, r, itm) {
    if (!this.obj[r]) this.obj[r] = {};
    this.obj[r][c] = itm;

  }
  get(c, r) {
    if (!this.obj[r]) return (undefined);
    return (this.obj[r][c]);
  }


  forEach(fn) {
    for (var r in this.obj) {
      for (var c in this.obj[r]) {
        fn(this.obj[r][c], c, r);
      }
    }
  }
  forEachFrom(sc,sr,ec,er,fn){
    if(sc>ec)forEachFrom(ec,sr,sc,er,fn);
    if(sr>er)forEachFrom(ec,er,sc,sr,fn);
    for (var r in this.obj) {
      if(sr<=r&&r<=er){
        for (var c in this.obj[r]) {
          if(sc<=c&&c<=ec)fn(this.obj[r][c], parseInt(c), parseInt(r));
        }
      }
    }
  }
  delete(c, r) {
    if (this.get(c, r)) {
      delete this.obj[r][c];
      if (!Object.keys(this.obj[r]).length) delete this.obj[r];
    }
  }
  clear() {

    this.obj = {};

  }
  clone() {
    var ret = new TwoDArrayWithNegativeIndex();
    this.forEach((a, x, y) => {
      ret.put(x, y, a.clone());
    });
    return (ret);
  }
}
