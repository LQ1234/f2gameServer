var WebGlHelperFunctions = {
  setBufferToRect: function(gl, fromx, fromy, tox, toy) {
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
      fromx, fromy,
      tox, fromy,
      fromx, toy,
      tox, fromy,
      fromx, toy,
      tox, toy
    ]), gl.STATIC_DRAW);
  },

  createProgram: function(gl, vertexShaderScript, fragmentShaderScript) {
    var program = gl.createProgram();
    gl.attachShader(program, this.createShader(gl, gl.VERTEX_SHADER, vertexShaderScript));
    gl.attachShader(program, this.createShader(gl, gl.FRAGMENT_SHADER, fragmentShaderScript));
    gl.linkProgram(program);
    if (gl.getProgramParameter(program, gl.LINK_STATUS)) return program;
    console.log(gl.getProgramInfoLog(program));
    gl.deleteProgram(program);
  },

  createShader: function(gl, type, source) {
    var shader = gl.createShader(type);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (gl.getShaderParameter(shader, gl.COMPILE_STATUS)) return shader;
    console.log(gl.getShaderInfoLog(shader));
    gl.deleteShader(shader);
  }
}

function webGlINIT() {
  initShadeCanvas();
  initBlockCanvas();
  resetLineBuffer();
  initCombinedCanvas();
}


window.addEventListener("load", function() {
  globalInformation.blockdatacanvas = document.getElementById("blockdata");
  globalInformation.rendercanvas = document.getElementById("webglrender");
  globalInformation.shadecanvas = document.getElementById("shadeshader");
  globalInformation.combinedshadercanvas = document.getElementById("combinedshader");
});

function initBlockCanvas() {
  globalInformation.blockdatacanvas.width = globalInformation.worldsizechunks.x * 10;
  globalInformation.blockdatacanvas.height = globalInformation.worldsizechunks.y * 10;

  var vertexShaderScript = `
    attribute vec2 blockloc;//in pixels
    attribute float blockdata;
    varying float toPassBlock;
    uniform vec2 u_textureSize;

    void main() {
      gl_PointSize =1.0;
      toPassBlock=blockdata;

      gl_Position = vec4((blockloc*2./u_textureSize-1.), 0, 1);
    }
    `;
  var fragmentShaderScript = `
    precision highp float;

    varying float toPassBlock;

    void main() {
      gl_FragColor=vec4(toPassBlock/255.,0,0,1);
    }
    `;

  var gl = globalInformation.blockdatacanvas.getContext("webgl", {
    alpha: false,
    antialias: false,
    preserveDrawingBuffer: true
  });
  gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
  gl.pixelStorei(gl.UNPACK_COLORSPACE_CONVERSION_WEBGL, false);

  var program = WebGlHelperFunctions.createProgram(gl, vertexShaderScript, fragmentShaderScript);
  gl.useProgram(program);

  var locations = {
    blockloc: gl.getAttribLocation(program, "blockloc"), //location of vertex of objects (for vertex shader)
    blockdata: gl.getAttribLocation(program, "blockdata"), //location of textures in objects (for vertex shader)
    textureSize: gl.getUniformLocation(program, "u_textureSize"), //size of texture (for fragment shader)
  }
  gl.uniform2f(locations.textureSize, globalInformation.blockdatacanvas.width, globalInformation.blockdatacanvas.height); //set texture size
  gl.viewport(0, 0, globalInformation.blockdatacanvas.width, globalInformation.blockdatacanvas.height);
  gl.clearColor(0.0, 0, 0.0, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

};


function setBlockCanvas(blocklocarr, blockdataarr) {
  var gl = globalInformation.blockdatacanvas.getContext("webgl", {
    alpha: false,
    antialias: false,
    preserveDrawingBuffer: true
  });
  var program = gl.getParameter(gl.CURRENT_PROGRAM);

  var locations = {
    blockloc: gl.getAttribLocation(program, "blockloc"),
    blockdata: gl.getAttribLocation(program, "blockdata"),
    textureSize: gl.getUniformLocation(program, "u_textureSize"),
  }

  var blocklocBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, blocklocBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, new Uint16Array(blocklocarr), gl.DYNAMIC_DRAW);
  gl.enableVertexAttribArray(locations.blockloc);
  gl.vertexAttribPointer(locations.blockloc, 2, gl.UNSIGNED_SHORT, false, 0, 0);

  var blockdataBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, blockdataBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, new Uint16Array(blockdataarr), gl.DYNAMIC_DRAW);
  gl.enableVertexAttribArray(locations.blockdata);
  gl.vertexAttribPointer(locations.blockdata, 1, gl.UNSIGNED_SHORT, false, 0, 0);

  var primitiveType = gl.POINTS;
  var offset = 0;
  var count = blockdataarr.length;
  gl.drawArrays(primitiveType, offset, count);
}

function addLineToBuffer(x1, y1, x2, y2) {
  const A1x = x1,
    A1y = y1,
    A2x = x1,
    A2y = y1,
    B1x = x2,
    B1y = y2,
    B2x = x2,
    B2y = y2,
    C1x = x1,
    C1y = y1,
    C2x = x2,
    C2y = y2,
    D1x = x2,
    D1y = y2,
    D2x = x1,
    D2y = y1;

  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = A1x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = A1y;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = A2x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = A2y;

  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = B1x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = B1y;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = B2x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = B2y;

  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = C1x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = C1y;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = C2x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = C2y;


  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = D1x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = D1y;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = D2x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = D2y;

  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = B1x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = B1y;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = B2x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = B2y;

  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = C1x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = C1y;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = C2x;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = C2y;
}

function resetLineBuffer() {
  globalInformation.shadeTrianglesPointsIndex = 0;
  //For efficiency
  const mn = -1,
    mx = 1;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = globalInformation.shadeMarkAsBg;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mn;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mn;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = 0;

  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = globalInformation.shadeMarkAsBg;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mx;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mn;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = 0;

  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = globalInformation.shadeMarkAsBg;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mn;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mx;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = 0;


  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = globalInformation.shadeMarkAsBg;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mx;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mx;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = 0;

  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = globalInformation.shadeMarkAsBg;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mx;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mn;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = 0;

  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = globalInformation.shadeMarkAsBg;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mn;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = mx;
  globalInformation.shadeTrianglesPoints[globalInformation.shadeTrianglesPointsIndex++] = 0;
}

function addLightToBuffer(x, y, r, g, b) {
  globalInformation.lightPoints.push(x, y);
  globalInformation.lightColors.push(r, g, b);
}

function resetLightBuffer() {
  globalInformation.lightPoints = [];
  globalInformation.lightColors = [];
}

function initShadeCanvas() {
  globalInformation.shadecanvas.width = globalInformation.vpblocks.x * globalInformation.renderscale;
  globalInformation.shadecanvas.height = globalInformation.vpblocks.y * globalInformation.renderscale;
  var vertexShaderScript = `


  attribute vec4 linepos;
  uniform vec2 lightpos;

  varying float isBg;
  void main() {

    if(linepos.x==float(${globalInformation.shadeMarkAsBg})){
      isBg=1.;
      gl_Position = vec4(linepos.gb*2.-1.,0, 1);

    }else if(lightpos.x==float(${globalInformation.shadeMarkAsSkylight})){
      vec2 lA=linepos.xy;
      vec2 lB=linepos.zw;
      vec2 pos;
      if(lA.x==lB.x&&lA.y==lB.y){
        pos=lA;
      }else{
        pos=vec2(lA.x,0);
      }
      isBg=0.;
      gl_Position = vec4(pos*2.-1.,0, 1);
    }else{
      vec2 lA=linepos.xy;
      vec2 lB=linepos.zw;
      vec2 pos;
      if(lA.x==lB.x&&lA.y==lB.y){
        pos=lA;
      }else{
        pos=lA+normalize(lA-lightpos)*10000.;
      }
      isBg=0.;
      gl_Position = vec4(pos*2.-1.,0, 1);

    }
  }
  `;
  var fragmentShaderScript = `
  precision highp float;
  uniform vec2 lightpos;
  uniform vec3 lightcolor;
  uniform vec2 blocksize;
  uniform sampler2D prevrender;
  uniform sampler2D skylightdata;

  varying float isBg;

  void main() {
    vec2 pixelPos=gl_FragCoord.rg/vec2(${ globalInformation.shadecanvas.width},${globalInformation.shadecanvas.height});
    vec3 currcolor=texture2D(prevrender,pixelPos).rgb;

    if(isBg==1.){
      if(lightpos.x==float(${globalInformation.shadeMarkAsSkylight})){
        currcolor+=lightcolor*texture2D(skylightdata,pixelPos).r;
      }else{
        float dist=length((lightpos-pixelPos)/blocksize);

        currcolor+=vec3(lightcolor*.5/(pow(dist/3.5,1.5)+.4));
      }
    }
    gl_FragColor=vec4(currcolor,1.);

  }
  `;
  var gl = globalInformation.shadecanvas.getContext("webgl", {
    alpha: true,
    antialias: true,
    depth: false,
  });

  var program = WebGlHelperFunctions.createProgram(gl, vertexShaderScript, fragmentShaderScript);
  gl.useProgram(program);
  var locations = {
    linepos: gl.getAttribLocation(program, "linepos"),
  }
  globalInformation.shadeTrianglesWebglObjs.pointsVtxBfr = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, globalInformation.shadeTrianglesWebglObjs.pointsVtxBfr);
  gl.bufferData(gl.ARRAY_BUFFER, globalInformation.shadeTrianglesPoints, gl.DYNAMIC_DRAW);
  gl.enableVertexAttribArray(locations.linepos);
  gl.vertexAttribPointer(locations.linepos, 4, gl.FLOAT, false, 0, 0);



  gl.viewport(0, 0, globalInformation.shadecanvas.width, globalInformation.shadecanvas.height);
  gl.clearColor(0.0, 0, 0.0, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
}

function renderShadeCanvas() {
  var gl = globalInformation.shadecanvas.getContext("webgl", {
    alpha: true,
    antialias: true,
    depth: false,

  });

  //gl.disable(gl.DEPTH_TEST);

  var program = gl.getParameter(gl.CURRENT_PROGRAM);

  var locations = {

    linepos: gl.getAttribLocation(program, "linepos"),

    lightpos: gl.getUniformLocation(program, "lightpos"),
    lightcolor: gl.getUniformLocation(program, "lightcolor"),
    blocksize: gl.getUniformLocation(program, "blocksize"),
    prevrender: gl.getUniformLocation(program, "prevrender"),
    skylightdata: gl.getUniformLocation(program, "skylightdata"),

  }
  gl.uniform2f(locations.blocksize, 1 / globalInformation.vpblocks.x, 1 / globalInformation.vpblocks.y);

  gl.bindBuffer(gl.ARRAY_BUFFER, globalInformation.shadeTrianglesWebglObjs.pointsVtxBfr);
  gl.bufferSubData(gl.ARRAY_BUFFER, 0, globalInformation.shadeTrianglesPoints);
  gl.bindTexture(gl.TEXTURE_2D, globalInformation.shadeTrianglesWebglObjs.intermedTexA);


  {
    if (!globalInformation.shadeTrianglesWebglObjs.intermedTexA) {
      globalInformation.shadeTrianglesWebglObjs.intermedTexA = gl.createTexture();
      gl.bindTexture(gl.TEXTURE_2D, globalInformation.shadeTrianglesWebglObjs.intermedTexA);

      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    } else {
      gl.bindTexture(gl.TEXTURE_2D, globalInformation.shadeTrianglesWebglObjs.intermedTexA);
    }
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA,
      globalInformation.shadecanvas.width, globalInformation.shadecanvas.height, 0,
      gl.RGBA, gl.UNSIGNED_BYTE, null);

    if (!globalInformation.shadeTrianglesWebglObjs.fbA) {
      globalInformation.shadeTrianglesWebglObjs.fbA = gl.createFramebuffer();
      gl.bindFramebuffer(gl.FRAMEBUFFER, globalInformation.shadeTrianglesWebglObjs.fbA);
      gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, globalInformation.shadeTrianglesWebglObjs.intermedTexA, 0);
    }


    if (!globalInformation.shadeTrianglesWebglObjs.intermedTexB) {
      globalInformation.shadeTrianglesWebglObjs.intermedTexB = gl.createTexture();
      gl.bindTexture(gl.TEXTURE_2D, globalInformation.shadeTrianglesWebglObjs.intermedTexB);


      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    } else {
      gl.bindTexture(gl.TEXTURE_2D, globalInformation.shadeTrianglesWebglObjs.intermedTexB);

    }
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA,
      globalInformation.shadecanvas.width, globalInformation.shadecanvas.height, 0,
      gl.RGBA, gl.UNSIGNED_BYTE, null);
    if (!globalInformation.shadeTrianglesWebglObjs.fbB) {
      globalInformation.shadeTrianglesWebglObjs.fbB = gl.createFramebuffer();
      gl.bindFramebuffer(gl.FRAMEBUFFER, globalInformation.shadeTrianglesWebglObjs.fbB);
      gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, globalInformation.shadeTrianglesWebglObjs.intermedTexB, 0);
    }


    {
      var skylightTex = gl.createTexture();
      gl.activeTexture(gl.TEXTURE2);
      gl.bindTexture(gl.TEXTURE_2D, skylightTex);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
      gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA,
        gl.RGBA, gl.UNSIGNED_BYTE, globalInformation.skylightdatacanvas);
      gl.uniform1i(locations.skylightdata, 2);
    }


    var passes = globalInformation.lightPoints.length / 2;
    for (var pass = 0; pass < passes; pass++) {

      if (pass % 2 == 0) {
        gl.bindFramebuffer(gl.FRAMEBUFFER, globalInformation.shadeTrianglesWebglObjs.fbB);
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, globalInformation.shadeTrianglesWebglObjs.intermedTexA);
        gl.uniform1i(locations.prevrender, 0);
      } else {
        gl.bindFramebuffer(gl.FRAMEBUFFER, globalInformation.shadeTrianglesWebglObjs.fbA);
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, globalInformation.shadeTrianglesWebglObjs.intermedTexB);
        gl.uniform1i(locations.prevrender, 0);
      }

      if (pass == passes - 1) gl.bindFramebuffer(gl.FRAMEBUFFER, null);

      gl.uniform2f(locations.lightpos, globalInformation.lightPoints[pass * 2], globalInformation.lightPoints[pass * 2 + 1]);
      gl.uniform3f(locations.lightcolor, globalInformation.lightColors[pass * 3], globalInformation.lightColors[pass * 3 + 1], globalInformation.lightColors[pass * 3 + 2]);

      gl.drawArrays(gl.TRIANGLES, 0, globalInformation.shadeTrianglesPointsIndex / 4);
    }

  }

}


function initCombinedCanvas() {
  globalInformation.combinedshadercanvas.width = globalInformation.vpblocks.x * globalInformation.renderscale;
  globalInformation.combinedshadercanvas.height = globalInformation.vpblocks.y * globalInformation.renderscale;
  var vertexShaderScript = `
  attribute vec2 pos;

  void main() {
      gl_Position = vec4(pos*2.-1.,0, 1);
  }
  `;
  var fragmentShaderScript = `
  precision highp float;

  uniform sampler2D shadeData;
  uniform sampler2D geometryData;
  uniform sampler2D geometryBGData;

  void main() {
    vec2 pixelPos=gl_FragCoord.rg/vec2(${ globalInformation.combinedshadercanvas.width},${globalInformation.combinedshadercanvas.height});
    pixelPos=vec2(pixelPos.x,1.-pixelPos.y);

    //texture2D(geometryBGData,pixelPos).rgb

    vec3 color=vec3(0,0,0);

    vec4 shadeColor=texture2D(shadeData,pixelPos);
    vec4 shadeColorAdj=shadeColor*.7+vec4(1,1,1,1)*.3;
    vec4 geometryColor=texture2D(geometryData,pixelPos);
    vec4 geometryBGColor=texture2D(geometryBGData,pixelPos);
    if(geometryColor.a>.0){
      color+=geometryColor.rgb;
    }else{
      if(geometryBGColor.a>0.){
        color+=shadeColorAdj.rgb*.6;
        color+=geometryBGColor.rgb*.8-vec3(1.,1.,1)*.3;
      }else{
        color+=shadeColorAdj.rgb*.6;
        color+=vec3(1.,1.,1.)*.4;
      }
    }

    gl_FragColor=vec4(color,1.);

  }
  `;
  var gl = globalInformation.combinedshadercanvas.getContext("webgl", {
    alpha: true,
    antialias: true,
    depth: false,
  });

  var program = WebGlHelperFunctions.createProgram(gl, vertexShaderScript, fragmentShaderScript);
  gl.useProgram(program);
  var locations = {
    pos: gl.getAttribLocation(program, "pos"),

    shadeData: gl.getUniformLocation(program, "shadeData"),
    geometryData: gl.getUniformLocation(program, "geometryData"),
    geometryBGData: gl.getUniformLocation(program, "geometryBGData"),
  }

  var posBuf = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, posBuf);
  WebGlHelperFunctions.setBufferToRect(gl, 0, 0, 1, 1);

  gl.enableVertexAttribArray(locations.pos);

  gl.vertexAttribPointer(locations.pos, 2, gl.FLOAT, false, 0, 0);

  globalInformation.combinedshadercanvasWebglObjs.shadeDataTex = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, globalInformation.combinedshadercanvasWebglObjs.shadeDataTex);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

  globalInformation.combinedshadercanvasWebglObjs.geometryDataTex = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, globalInformation.combinedshadercanvasWebglObjs.geometryDataTex);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

  globalInformation.combinedshadercanvasWebglObjs.geometryBGDataTex = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, globalInformation.combinedshadercanvasWebglObjs.geometryBGDataTex);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

  gl.viewport(0, 0, globalInformation.combinedshadercanvas.width, globalInformation.combinedshadercanvas.height);
  gl.clearColor(0.0, 0, 0.0, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
}


function renderCombinedCanvas() {
  var gl = globalInformation.combinedshadercanvas.getContext("webgl", {
    alpha: true,
    antialias: true,
    depth: false,

  });

  //gl.disable(gl.DEPTH_TEST);

  var program = gl.getParameter(gl.CURRENT_PROGRAM);

  var locations = {
    pos: gl.getAttribLocation(program, "pos"),

    shadeData: gl.getUniformLocation(program, "shadeData"),
    geometryData: gl.getUniformLocation(program, "geometryData"),
    geometryBGData: gl.getUniformLocation(program, "geometryBGData"),
  }




  gl.activeTexture(gl.TEXTURE0);
  gl.bindTexture(gl.TEXTURE_2D, globalInformation.combinedshadercanvasWebglObjs.shadeDataTex);
  gl.uniform1i(locations.shadeData, 0);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, globalInformation.shadecanvas);


  gl.activeTexture(gl.TEXTURE1);
  gl.bindTexture(gl.TEXTURE_2D, globalInformation.combinedshadercanvasWebglObjs.geometryDataTex);
  gl.uniform1i(locations.geometryData, 1);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, globalInformation.foregroundrendercanvas);


  gl.activeTexture(gl.TEXTURE2);
  gl.bindTexture(gl.TEXTURE_2D, globalInformation.combinedshadercanvasWebglObjs.geometryBGDataTex);
  gl.uniform1i(locations.geometryBGData, 2);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, globalInformation.backgroundrendercanvas);


  gl.drawArrays(gl.TRIANGLES, 0, 6);

}
