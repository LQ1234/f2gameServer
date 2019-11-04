function sizeofenum() {
  return (Object.keys(inp).length);
}

function addEnumKeys(inp, enu, value) {
  Object.keys(enu).forEach((x) => {
    inp.push([x, value]);
  });
  return (inp);
}

function CPPEnum(asStr) {
  var ret = {};
  var asArr = asStr.split(",");
  for (var i = 0; i < asArr.length; i++) {
    ret[asArr[i].trim()] = i;
  }
  return (ret)
}

function rltEnum(enu) {
  var ret = [];
  for (n in enu) {
    ret[enu[n]] = n;
  }
  return (ret)
}

var globalInformation = {
  itemType: CPPEnum("SHOVEL, AXE, PICKAXE, GUN, POTION, GRASSPIECE, DIRTPIECE, STONEPIECE"),
  blockType: CPPEnum("NULLBLOCK, STONEBLOCK, PLANKBLOCK, STONEBRICKBLOCK, LOGVBLOCK, LOGHBLOCK, GLASSBLOCK, TORCHBLOCK, LEAFBLOCK"),

  basicrendercanvas: null,
  blockdatacanvas: null,
  rendercanvas: null,
  shadecanvas:null,


  fullLinebufferLength:10000,
  fullLinebufferIndex:0,
  fullLinebuffer: new Float32Array(10000),

  partialLinebufferLength:1000,

  renderscale: 30,
  framespersec: 45,
  thisplayer: {},
  renderlist: [],
  vpblocks: {
    x: 26.6,
    y: 15
  },
  vpchunks: {
    x: 2,
    y: 2
  },
  cam: {
    x: 0,
    y: 0
  },
  worldsizechunks: {
    x: 40,
    y: 20
  }
}
console.log(globalInformation.fullLinebuffer);
