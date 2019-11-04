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



}


window.addEventListener("load", function() {
  globalInformation.blockdatacanvas = document.getElementById("blockdata");
  globalInformation.rendercanvas = document.getElementById("webglrender");
  globalInformation.shadecanvas = document.getElementById("webglshader");
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

function resetLineBuffer() {
  globalInformation.fullLinebufferIndex = 0;
  globalInformation.fullLinebuffer.fill(0);
}

function addLineToBuffer(x1, y1, x2, y2) { // 0 to 1
  globalInformation.fullLinebuffer[globalInformation.fullLinebufferIndex++] = x1;
  globalInformation.fullLinebuffer[globalInformation.fullLinebufferIndex++] = y1;
  globalInformation.fullLinebuffer[globalInformation.fullLinebufferIndex++] = x2;
  globalInformation.fullLinebuffer[globalInformation.fullLinebufferIndex++] = y2;
}


function initShadeCanvas() {
  globalInformation.shadecanvas.width = globalInformation.vpblocks.x * globalInformation.renderscale;
  globalInformation.shadecanvas.height = globalInformation.vpblocks.y * globalInformation.renderscale;
  var vertexShaderScript = `
  uniform vec2 screensize;
  uniform vec2 cam;
  uniform float scale;

  attribute vec2 vertexPos;//0 to 1
  varying vec2 pixelPos;

  void main() {
    gl_Position = vec4((vertexPos*2.-1.), 0, 1);
    pixelPos = vertexPos;
  }
  `;
  var fragmentShaderScript = `
  precision highp float;

  varying vec2 pixelPos;

  uniform vec2 screensize;
  uniform vec2 cam;
  uniform float scale;

  uniform float partialLinebuffer[${globalInformation.partialLinebufferLength}];

  uniform sampler2D prevrender;

  float minimum_distance(vec2 v, vec2 w, vec2 p) {
    // Return minimum distance between line segment vw and point p
    float l2 = pow(length(v-w),2.);  // i.e. |w-v|^2 -  avoid a sqrt
    if (l2 == 0.0) return distance(p, v);   // v == w case
    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    // We clamp t from [0,1] to handle points outside the segment vw.
    float t = max(0., min(1., dot(p - v, w - v) / l2));
    vec2 projection = v + t * (w - v);  // Projection falls on the segment
    return distance(p, projection);
  }

  float twodcross(vec2 v,vec2 w){
    return(v.x*w.y-v.y*w.x);
  }
  //Following function derived from https://stackoverflow.com/a/565282
  bool doesintersect(vec2 p,vec2 a,vec2 q,vec2 b){
    vec2 r=a-p;
    vec2 s=b-q;
    float t = twodcross((q - p), s /twodcross(r, s));
    float u = twodcross((q - p), r / twodcross(r, s));
    return(0.<=t&&t<=1.&&0.<=u&&u<=1.);
  }

  void main() {
    gl_FragColor=texture2D(prevrender,pixelPos);

    for(int i=0;i<${globalInformation.partialLinebufferLength};i+=4){
      if(doesintersect(vec2(.5,.5),pixelPos,vec2(partialLinebuffer[i+0],partialLinebuffer[i+1]),vec2(partialLinebuffer[i+2],partialLinebuffer[i+3]))){
        gl_FragColor=vec4(0,1,1,1);

      }
      /*if(minimum_distance(vec2(partialLinebuffer[i],partialLinebuffer[i+1]),vec2(partialLinebuffer[i+2],partialLinebuffer[i+3]),pixelPos)<.002){
        gl_FragColor=vec4(0,1,1,1);
        break;
      }*/

    }
  }
  `;
  var gl = globalInformation.shadecanvas.getContext("webgl", {
    alpha: true,
    antialias: true,
  });

  var program = WebGlHelperFunctions.createProgram(gl, vertexShaderScript, fragmentShaderScript);
  gl.useProgram(program);

  var locations = {
    vertexPos: gl.getAttribLocation(program, "vertexPos"),

    screensize: gl.getUniformLocation(program, "screensize"),
    cam: gl.getUniformLocation(program, "cam"),
    scale: gl.getUniformLocation(program, "scale"),

    partialLinebuffer: gl.getUniformLocation(program, "partialLinebuffer"),

    prevrender: gl.getUniformLocation(program, "prevrender"),
  }

  var vertexBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
  WebGlHelperFunctions.setBufferToRect(gl, 0, 0, 1, 1);
  gl.enableVertexAttribArray(locations.vertexPos);
  gl.vertexAttribPointer(locations.vertexPos, 2, gl.FLOAT, false, 0, 0);


  gl.viewport(0, 0, globalInformation.shadecanvas.width, globalInformation.shadecanvas.height);
  gl.clearColor(0.0, 0, 0.0, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
}


function renderShadeCanvas() {
  var gl = globalInformation.shadecanvas.getContext("webgl", {
    alpha: true,
    antialias: true,
  });

  var program = gl.getParameter(gl.CURRENT_PROGRAM);

  var locations = {
    vertexPos: gl.getAttribLocation(program, "vertexPos"),

    screensize: gl.getUniformLocation(program, "screensize"),
    cam: gl.getUniformLocation(program, "cam"),
    scale: gl.getUniformLocation(program, "scale"),

    partialLinebuffer: gl.getUniformLocation(program, "partialLinebuffer"),

    prevrender: gl.getUniformLocation(program, "prevrender"),
  }

  gl.uniform2f(locations.screensize, globalInformation.rendercanvas.width, globalInformation.rendercanvas.height);
  gl.uniform2f(locations.cam, globalInformation.cam.x, globalInformation.cam.y);
  gl.uniform1f(locations.scale, globalInformation.renderscale);

  var neededPasses = Math.ceil(Math.min(globalInformation.fullLinebufferIndex, globalInformation.fullLinebufferLength) / globalInformation.partialLinebufferLength);

  var intermedTexA = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, intermedTexA);

  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA,
    globalInformation.shadecanvas.width, globalInformation.shadecanvas.height, 0,
    gl.RGBA, gl.UNSIGNED_BYTE, null);

  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);


  var intermedTexB = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, intermedTexB);

  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA,
    globalInformation.shadecanvas.width, globalInformation.shadecanvas.height, 0,
    gl.RGBA, gl.UNSIGNED_BYTE, null);

  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);


  const fb = gl.createFramebuffer();
  gl.bindFramebuffer(gl.FRAMEBUFFER, fb);

  for (var pass = 0; pass < neededPasses; pass++) {
    //console.log(new Float32Array(globalInformation.fullLinebuffer,pass*globalInformation.partialLinebufferLength,globalInformation.partialLinebufferLength));
    if(pass%2==0){
      gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, intermedTexB, 0);
      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, intermedTexA);
      gl.uniform1i(locations.prevrender, 0);
    }else{
      gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, intermedTexA, 0);
      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, intermedTexB);
      gl.uniform1i(locations.prevrender, 0);
    }
    gl.uniform1fv(locations.partialLinebuffer,globalInformation.fullLinebuffer.slice(pass*globalInformation.partialLinebufferLength,(pass+1)*globalInformation.partialLinebufferLength));
    if(pass==neededPasses-1)gl.bindFramebuffer(gl.FRAMEBUFFER,null);
    gl.drawArrays(gl.TRIANGLES, 0, 6);
  }

}



function initrenderCanvas() {

  globalInformation.rendercanvas.width = globalInformation.vpblocks.x * globalInformation.renderscale;
  globalInformation.rendercanvas.height = globalInformation.vpblocks.y * globalInformation.renderscale;


  var vertexShaderScript = `

    uniform vec2 screensize;

    uniform vec2 cam;
    uniform float scale;

    uniform vec2 linebuffersize;
    uniform sampler2D linebuffer;

    uniform vec2 blockdatasize;
    uniform sampler2D blockdata;

    attribute vec2 vertexPos;//0 to 1
    varying vec2 pixelPos;

    void main() {
      gl_Position = vec4((vertexPos*2.-1.)*vec2(1.,-1.), 0, 1);
      pixelPos = vertexPos;
    }
    `;
  var fragmentShaderScript = `
    precision highp float;
    varying vec2 pixelPos;

    uniform vec2 screensize;

    uniform vec2 cam;
    uniform float scale;

    uniform vec2 linebuffersize;
    uniform sampler2D linebuffer;

    uniform vec2 blockdatasize;
    uniform sampler2D blockdata;

    vec4 lineDataAt(int indx){
      int yp=indx/${globalInformation.linedatawidth};
      float xp=mod(float(indx),${globalInformation.linedatawidth.toFixed(1)});
      if((vec2(xp,yp)/linebuffersize).x>1.){
        return(vec4(1,1,1,1));
      }
      return(texture2D(linebuffer,vec2(xp,yp)/linebuffersize));
    }
    int vec3toint(vec3 v){
      return(int(65536.*255.*v.r+256.*255.*v.g+255.*v.b));
    }
    int getMaxLineIndexAt(float x,float y){
      int xg=int(x*${globalInformation.linebuffergridx.toFixed(1)});
      int yg=int(y*${globalInformation.linebuffergridy.toFixed(1)});
      return(vec3toint(lineDataAt(xg+yg*${globalInformation.linebuffergridx}).rgb)*2+
             ${(globalInformation.linebuffergridx)*(globalInformation.linebuffergridy)});
    }
    int getMaxLineIndexOf(int xg,int yg){
      return(vec3toint(lineDataAt(xg+yg*${globalInformation.linebuffergridx}).rgb)*2+
             ${(globalInformation.linebuffergridx)*(globalInformation.linebuffergridy)});
    }
    int getMinLineIndexAt(float x,float y){
      int xg=int(x*${globalInformation.linebuffergridx.toFixed(1)});
      int yg=int(y*${globalInformation.linebuffergridy.toFixed(1)});
      if(xg==0&&yg==0)return(${(globalInformation.linebuffergridx)*(globalInformation.linebuffergridy)});

      return(vec3toint(lineDataAt(xg+yg*${globalInformation.linebuffergridx}-1).rgb)*2+
             ${(globalInformation.linebuffergridx)*(globalInformation.linebuffergridy)});
    }
    int getMinLineIndexOf(int xg,int yg){
      if(xg==0&&yg==0)return(${(globalInformation.linebuffergridx)*(globalInformation.linebuffergridy)});

      return(vec3toint(lineDataAt(xg+yg*${globalInformation.linebuffergridx}-1).rgb)*2+
             ${(globalInformation.linebuffergridx)*(globalInformation.linebuffergridy)});
    }
    //Following function is from https://stackoverflow.com/a/1501725, for line-point distance
    float minimum_distance(vec2 v, vec2 w, vec2 p) {
      // Return minimum distance between line segment vw and point p
      float l2 = pow(length(v-w),2.);  // i.e. |w-v|^2 -  avoid a sqrt
      if (l2 == 0.0) return distance(p, v);   // v == w case
      // Consider the line extending the segment, parameterized as v + t (w - v).
      // We find projection of point p onto the line.
      // It falls where t = [(p-v) . (w-v)] / |w-v|^2
      // We clamp t from [0,1] to handle points outside the segment vw.
      float t = max(0., min(1., dot(p - v, w - v) / l2));
      vec2 projection = v + t * (w - v);  // Projection falls on the segment
      return distance(p, projection);
    }

    float twodcross(vec2 v,vec2 w){
      return(v.x*w.y-v.y*w.x);
    }
    //Following function is from https://stackoverflow.com/a/565282, for line-line intersection
    bool doesintersect(vec2 p,vec2 a,vec2 q,vec2 b){
      vec2 r=a-p;
      vec2 s=b-q;
      float t = twodcross((q - p), s /twodcross(r, s));
      float u = twodcross((q - p), r / twodcross(r, s));
      return(0.<=t&&t<=1.&&0.<=u&&u<=1.);
    }

    bool raycastTo(vec2 pt1,vec2 pt2){
      vec2 mx=max(pt1,pt2);
      vec2 mn=min(pt1,pt2);

      for(int xgrid=0;xgrid<${globalInformation.linebuffergridx};xgrid++){
        if(int(mn.x)<=xgrid&&xgrid<=int(mx.x)){
          float x_min=max(mn.x,(float(xgrid)  )/${globalInformation.linebuffergridx.toFixed(1)});
          float x_max=min(mx.x,(float(xgrid)+1.)/${globalInformation.linebuffergridx.toFixed(1)});
          float x_min_y=pt1.y+(pt2.y-pt1.y)/(pt2.x-pt1.x)*(x_min-pt1.x);
          float x_max_y=pt1.y+(pt2.y-pt1.y)/(pt2.x-pt1.x)*(x_max-pt1.x);
          float y_min=min(x_min_y,x_max_y);
          float y_max=max(x_min_y,x_max_y);

          for(int ygrid=0;ygrid<${globalInformation.linebuffergridy};ygrid++){
            if(int(y_min)<=ygrid&&ygrid<=int(y_max)){
              vec2 gridPos=vec2(xgrid,ygrid)/vec2(${globalInformation.linebuffergridx.toFixed(1)},${globalInformation.linebuffergridy.toFixed(1)});

              int lineIndexMin=getMinLineIndexOf(xgrid,ygrid);
              int lineIndexMax=getMaxLineIndexOf(xgrid,ygrid);

              for(int i=0;i<${globalInformation.linebuffermaxperbox*2};i+=2){
                if(lineIndexMin+i>=lineIndexMax)break;
                vec2 lineStart =(lineDataAt(lineIndexMin+i  ).xy*vec2(1.,-1.)+vec2(0.,1.))/vec2(${globalInformation.linebuffergridx},${globalInformation.linebuffergridy})+gridPos;
                vec2 lineEnd   =(lineDataAt(lineIndexMin+i+1).xy*vec2(1.,-1.)+vec2(0.,1.))/vec2(${globalInformation.linebuffergridx},${globalInformation.linebuffergridy})+gridPos;
                if(doesintersect(lineStart,lineEnd,vec2(.5,.5),pixelPos)){
                  return(true);
                  //gl_FragColor=vec4(1,1,1,1);

                }
                /*
                if(minimum_distance(lineStart,lineEnd,pixelPos)<.002){
                  gl_FragColor=vec4(1,1,1,1);
                }*/

              }

            }
          }
        }
      }
      return(false);
    }
    vec4 getBlockOnScreen(vec2 pixelPos){
      return(texture2D(blockdata,(vec2(cam.x,blockdatasize.y-cam.y)+(pixelPos*1.-.5)*screensize/scale+vec2(-1,1))/blockdatasize));
    }
    void main() {
      vec4 thiscol=texture2D(blockdata,(vec2(cam.x,blockdatasize.y-cam.y)+(pixelPos*1.-.5)*screensize/scale+vec2(-1,1))/blockdatasize);
      gl_FragColor=vec4(0,0,0,1);

      if(thiscol.r*255.>0.){
        gl_FragColor=vec4(1,1,1,1);
      }
      /*
      int lineIndexMin=getMinLineIndexAt(pixelPos.x,1.-pixelPos.y);
      int lineIndexMax=getMaxLineIndexAt(pixelPos.x,1.-pixelPos.y);


      vec2 gridPos=floor(pixelPos*vec2(${globalInformation.linebuffergridx},${globalInformation.linebuffergridy}))/
                                  vec2(${globalInformation.linebuffergridx},${globalInformation.linebuffergridy});

      for(int i=0;i<${globalInformation.linebuffermaxperbox*2};i+=2){
        if(lineIndexMin+i>=lineIndexMax)break;
        vec2 lineStart =(lineDataAt(lineIndexMin+i  ).xy*vec2(1.,-1.)+vec2(0.,1.))/vec2(${globalInformation.linebuffergridx},${globalInformation.linebuffergridy})+gridPos;
        vec2 lineEnd   =(lineDataAt(lineIndexMin+i+1).xy*vec2(1.,-1.)+vec2(0.,1.))/vec2(${globalInformation.linebuffergridx},${globalInformation.linebuffergridy})+gridPos;
        if(doesintersect(lineStart,lineEnd,vec2(.5,.5),pixelPos)){
          gl_FragColor=vec4(1,1,1,1);

        }

        if(minimum_distance(lineStart,lineEnd,pixelPos)<.002){
          gl_FragColor=vec4(1,1,1,1);
        }

      }*/

      if(raycastTo(vec2(.5,.5),pixelPos)){
        gl_FragColor=vec4(1,1,1,1);

      }
      //gl_FragColor=vec4(getMaxLineIndexAt(pixelPos.x,pixelPos.y),1,1,1);

      if(distance(vec2(.5,.5)*screensize,pixelPos*screensize)<10.){
        gl_FragColor=vec4(1.-gl_FragColor.r,1.-gl_FragColor.g,1.-gl_FragColor.b,1);
      }
      //gl_FragColor=vec4(1,1,1,1);
    }
    `;

  var gl = globalInformation.rendercanvas.getContext("webgl", {
    alpha: true,
    antialias: true,
  });


  var program = WebGlHelperFunctions.createProgram(gl, vertexShaderScript, fragmentShaderScript);
  gl.useProgram(program);

  var locations = {
    screensize: gl.getUniformLocation(program, "screensize"),
    cam: gl.getUniformLocation(program, "cam"),
    scale: gl.getUniformLocation(program, "scale"),

    linebuffersize: gl.getUniformLocation(program, "linebuffersize"),
    linebuffer: gl.getUniformLocation(program, "linebuffer"),

    blockdatasize: gl.getUniformLocation(program, "blockdatasize"),
    blockdata: gl.getUniformLocation(program, "blockdata"),

    vertexPos: gl.getUniformLocation(program, "vertexPos"),
  }

  var vertexBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
  WebGlHelperFunctions.setBufferToRect(gl, 0, 0, 1, 1);
  gl.enableVertexAttribArray(locations.vertexPos);
  gl.vertexAttribPointer(locations.vertexPos, 2, gl.FLOAT, false, 0, 0);


  gl.viewport(0, 0, globalInformation.rendercanvas.width, globalInformation.rendercanvas.height);
  gl.clearColor(0.0, 0, 0.0, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
}

function renderRenderCanvas() {
  var gl = globalInformation.rendercanvas.getContext("webgl", {
    alpha: true,
    antialias: true,
  });
  var program = gl.getParameter(gl.CURRENT_PROGRAM);

  var locations = {
    screensize: gl.getUniformLocation(program, "screensize"),
    cam: gl.getUniformLocation(program, "cam"),
    scale: gl.getUniformLocation(program, "scale"),

    linebuffersize: gl.getUniformLocation(program, "linebuffersize"),
    linebuffer: gl.getUniformLocation(program, "linebuffer"),

    blockdatasize: gl.getUniformLocation(program, "blockdatasize"),
    blockdata: gl.getUniformLocation(program, "blockdata"),

    vertexPos: gl.getUniformLocation(program, "vertexPos"),
  }
  gl.uniform2f(locations.screensize, globalInformation.rendercanvas.width, globalInformation.rendercanvas.height);
  gl.uniform2f(locations.cam, globalInformation.cam.x, globalInformation.cam.y);
  gl.uniform1f(locations.scale, globalInformation.renderscale);

  linebuffercanvas = bufferToCanvas();
  gl.uniform2f(locations.linebuffersize, linebuffercanvas.width, linebuffercanvas.height); {

    var texture = gl.createTexture();
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.uniform1i(locations.linebuffer, 0);

    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, linebuffercanvas);

    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);


  }

  gl.uniform2f(locations.blockdatasize, globalInformation.blockdatacanvas.width, globalInformation.blockdatacanvas.height); {
    var texture = gl.createTexture();
    gl.activeTexture(gl.TEXTURE1);
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.uniform1i(locations.blockdata, 1);

    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, globalInformation.blockdatacanvas);

    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);


  }
  gl.drawArrays(gl.TRIANGLES, 0, 6);

}
