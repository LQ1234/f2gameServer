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

var globalInformation = (()=>{
  var maxLines=10000;
  return({
    itemType: CPPEnum("SHOVEL, AXE, PICKAXE, GUN, POTION, GRASSPIECE, DIRTPIECE, STONEPIECE"),
    blockType: CPPEnum("NULLBLOCK, STONEBLOCK, PLANKBLOCK, STONEBRICKBLOCK, LOGVBLOCK, LOGHBLOCK, GLASSBLOCK, TORCHBLOCK, LEAFBLOCK"),

    basicrendercanvas: null,
    backgroundrendercanvas: null,
    foregroundrendercanvas: null,

    combinedshadercanvas:null,
    combinedshadercanvasWebglObjs:{},


    blockdatacanvas: null,
    rendercanvas: null,
    shadecanvas:null,
    skylightdatacanvas:null,
    lightPoints: [],
    lightColors: [],

    debug:{mp2:{x:0,y:0}},

    shadeTrianglesPoints: new Float32Array(maxLines*4*3*2),//4 per point, 3 per triangle, 2 per lines
    shadeTrianglesPointsIndex: 0,
    shadeTrianglesWebglObjs:{},
    shadeTrianglesPointsVtxBfr:null,
    shadeMarkAsBg:-1234567890,//'magic number' to mark vertex as bg
    shadeMarkAsSkylight:-531412421,//'magic number' to mark light as skylight

    renderscale: 32,
    framespersec: 450,
    currentframecount: 0,
    realframepersec: 0,
    thisplayer: {},

    renderlist: [],
    rendershadelist: [],
    renderslightlist: [],
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
  });
})();
