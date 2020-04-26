window.onload = function(inp) {

  var renderItemInv = {}
  renderItemInv[globalInformation.itemType.SHOVEL] = function(ctx) {
    ctx.rotate(Math.PI / 5);
    ctx.fillStyle = 'rgb(255, 224, 140)';

    ctx.fillRect(-.05, 0, .1, .5);
    ctx.fillStyle = 'rgb(130, 130, 130)';
    ctx.fillRect(-.15, -.3, .3, .3);
    ctx.rotate(-Math.PI / 5);

  };
  renderItemInv[globalInformation.itemType.AXE] = function(ctx) {
    ctx.rotate(Math.PI / 5);
    ctx.fillStyle = 'rgb(255, 224, 140)';

    ctx.fillRect(-.05, -.3, .1, .8);
    ctx.fillStyle = 'rgb(130, 130, 130)';
    ctx.beginPath();
    ctx.moveTo(-.1, 0 - .15);
    ctx.lineTo(.3, .2 - .15);
    ctx.lineTo(.3, -.2 - .15);
    ctx.fill();
    ctx.rotate(-Math.PI / 5);
  };
  renderItemInv[globalInformation.itemType.PICKAXE] = function(ctx) {
    ctx.rotate(Math.PI / 5);
    ctx.fillStyle = 'rgb(255, 224, 140)';

    ctx.fillRect(-.05, -.2, .1, .7);
    ctx.fillStyle = 'rgb(130, 130, 130)';
    ctx.fillRect(-.3, -.3, .6, .2);
    ctx.rotate(-Math.PI / 5);
  };
  renderItemInv[globalInformation.itemType.GRASSPIECE] = function(ctx) {
    ctx.fillStyle = '#00ff44';
    ctx.beginPath();
    ctx.arc(0, 0, .2, 0, 2 * Math.PI);
    ctx.fill();

  };
  renderItemInv[globalInformation.itemType.DIRTPIECE] = function(ctx) {
    ctx.fillStyle = '#804d0f';
    ctx.beginPath();
    ctx.arc(0, 0, .2, 0, 2 * Math.PI);
    ctx.fill();

  };
  renderItemInv[globalInformation.itemType.STONEPIECE] = function(ctx) {
    ctx.fillStyle = '#696969';
    ctx.beginPath();
    ctx.arc(0, 0, .2, 0, 2 * Math.PI);
    ctx.fill();

  };


  globalInformation.basicrendercanvas = document.getElementById('basicrender');
  globalInformation.backgroundrendercanvas = document.getElementById('backgroundrender');
  globalInformation.foregroundrendercanvas = document.getElementById('foregroundrender');
  globalInformation.skylightdatacanvas = document.getElementById('skylightdata');

  var basicrendercanvasctx = globalInformation.basicrendercanvas.getContext('2d');


  var worldchunkdata = new Array(globalInformation.worldsizechunks.y);
  for (var i = 0; i < globalInformation.worldsizechunks.y; i++) {
    worldchunkdata[i] = new Array(globalInformation.worldsizechunks.x);
  }

  var worldbackgroundchunkdata = new Array(globalInformation.worldsizechunks.y);
  for (var i = 0; i < globalInformation.worldsizechunks.y; i++) {
    worldbackgroundchunkdata[i] = new Array(globalInformation.worldsizechunks.x);
  }

  var worldskylightdata = new Array(globalInformation.worldsizechunks.y);
  for (var i = 0; i < globalInformation.worldsizechunks.y; i++) {
    worldskylightdata[i] = new Array(globalInformation.worldsizechunks.x);
  }

  var worldblockdata = new TwoDArrayWithNegativeIndex();
  var movementkeyboard = [false, false, false, false];
  var mousepos = {
    x: 0,
    y: 0
  };
  var mousedown = false;
  //every item in the worldchunkdata 2d array is a client chunk piece array
  function rendermateriallayers() {
    function setup(canvas) {
      if (canvas.width != globalInformation.vpblocks.x * globalInformation.renderscale || canvas.height != globalInformation.vpblocks.y * globalInformation.renderscale) {
        canvas.width = globalInformation.vpblocks.x * globalInformation.renderscale;
        canvas.height = globalInformation.vpblocks.y * globalInformation.renderscale;
      }
      var ctx = canvas.getContext('2d');;
      ctx.setTransform(1, 0, 0, 1, 0, 0);;
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      ctx.translate(globalInformation.vpblocks.x / 2 * globalInformation.renderscale, globalInformation.vpblocks.y / 2 * globalInformation.renderscale);
      ctx.scale(globalInformation.renderscale, -globalInformation.renderscale);
      return ctx;
    }

    var backgroundrendercanvasctx = setup(globalInformation.backgroundrendercanvas);

    var foregroundrendercanvasctx = setup(globalInformation.foregroundrendercanvas);

    {
      backgroundrendercanvasctx.save();
      foregroundrendercanvasctx.save();

      var cxchunk = Math.floor(globalInformation.cam.x / 10);
      var cychunk = Math.floor(globalInformation.cam.y / 10);

      for (var ychunk = cychunk - globalInformation.vpchunks.y; ychunk <= cychunk + globalInformation.vpchunks.y; ychunk++) {
        for (var xchunk = cxchunk - globalInformation.vpchunks.x; xchunk <= cxchunk + globalInformation.vpchunks.x; xchunk++) {
          if (xchunk < 0 || ychunk < 0 || xchunk >= globalInformation.worldsizechunks.x || ychunk >= globalInformation.worldsizechunks.y) continue; {
            let thischunk = worldbackgroundchunkdata[ychunk][xchunk]
            if (thischunk) {
              backgroundrendercanvasctx.globalCompositeOperation = 'source-over';

              for (var i = 0; i < thischunk.length; i++) { //Draw filled in shapes
                if (!thischunk[i].isWhole) continue;
                if (thischunk[i].materialType == 0) {
                  backgroundrendercanvasctx.fillStyle = "#00ff44";
                } else if (thischunk[i].materialType == 1) {
                  backgroundrendercanvasctx.fillStyle = "#804d0f";
                } else if (thischunk[i].materialType == 2) {
                  backgroundrendercanvasctx.fillStyle = "#696969";
                }
                backgroundrendercanvasctx.beginPath();
                //console.log(thischunk[i]);

                backgroundrendercanvasctx.moveTo(thischunk[i].asVec[0] - globalInformation.cam.x, thischunk[i].asVec[1] - globalInformation.cam.y);
                for (var j = 0; j < thischunk[i].asVec.length; j += 2) {
                  backgroundrendercanvasctx.lineTo(thischunk[i].asVec[j] - globalInformation.cam.x, thischunk[i].asVec[j + 1] - globalInformation.cam.y);
                  //console.log(thischunk[i].asVec.length);
                }
                backgroundrendercanvasctx.closePath();
                backgroundrendercanvasctx.fill();
              }
              backgroundrendercanvasctx.globalCompositeOperation = 'destination-out';

              for (var i = 0; i < thischunk.length; i++) { //Erase holes
                if (thischunk[i].isWhole) continue;
                backgroundrendercanvasctx.fillStyle = "#ffffff";

                backgroundrendercanvasctx.beginPath();
                //console.log(thischunk[i]);

                backgroundrendercanvasctx.moveTo(thischunk[i].asVec[0] - globalInformation.cam.x, thischunk[i].asVec[1] - globalInformation.cam.y);
                for (var j = 0; j < thischunk[i].asVec.length; j += 2) {
                  backgroundrendercanvasctx.lineTo(thischunk[i].asVec[j] - globalInformation.cam.x, thischunk[i].asVec[j + 1] - globalInformation.cam.y);
                  //console.log(thischunk[i].asVec.length);
                }
                backgroundrendercanvasctx.closePath();
                backgroundrendercanvasctx.fill();
              }

            }
          } {
            let thischunk = worldchunkdata[ychunk][xchunk]
            if (thischunk) {
              foregroundrendercanvasctx.globalCompositeOperation = 'source-over';

              for (var i = 0; i < thischunk.length; i++) { //Draw filled in shapes
                if (!thischunk[i].isWhole) continue;
                if (thischunk[i].materialType == 0) {
                  foregroundrendercanvasctx.fillStyle = "#00ff44";
                } else if (thischunk[i].materialType == 1) {
                  foregroundrendercanvasctx.fillStyle = "#804d0f";
                } else if (thischunk[i].materialType == 2) {
                  foregroundrendercanvasctx.fillStyle = "#696969";
                }
                foregroundrendercanvasctx.beginPath();
                //console.log(thischunk[i]);

                foregroundrendercanvasctx.moveTo(thischunk[i].asVec[0] - globalInformation.cam.x, thischunk[i].asVec[1] - globalInformation.cam.y);
                for (var j = 0; j < thischunk[i].asVec.length; j += 2) {
                  foregroundrendercanvasctx.lineTo(thischunk[i].asVec[j] - globalInformation.cam.x, thischunk[i].asVec[j + 1] - globalInformation.cam.y);
                  //console.log(thischunk[i].asVec.length);
                }
                foregroundrendercanvasctx.closePath();
                foregroundrendercanvasctx.fill();

              }
              foregroundrendercanvasctx.globalCompositeOperation = 'destination-out';

              for (var i = 0; i < thischunk.length; i++) { //Erase holes
                if (thischunk[i].isWhole) continue;
                foregroundrendercanvasctx.fillStyle = "#ffffff";

                foregroundrendercanvasctx.beginPath();
                //console.log(thischunk[i]);

                foregroundrendercanvasctx.moveTo(thischunk[i].asVec[0] - globalInformation.cam.x, thischunk[i].asVec[1] - globalInformation.cam.y);
                for (var j = 0; j < thischunk[i].asVec.length; j += 2) {
                  foregroundrendercanvasctx.lineTo(thischunk[i].asVec[j] - globalInformation.cam.x, thischunk[i].asVec[j + 1] - globalInformation.cam.y);
                  //console.log(thischunk[i].asVec.length);
                }
                foregroundrendercanvasctx.closePath();
                foregroundrendercanvasctx.fill();
              }

            }
          }
        }
      }
      backgroundrendercanvasctx.restore();
      foregroundrendercanvasctx.restore();
    }
  }
  /*
  function basicrender() {

    if (globalInformation.basicrendercanvas.width != globalInformation.vpblocks.x * globalInformation.renderscale || globalInformation.basicrendercanvas.height != globalInformation.vpblocks.y * globalInformation.renderscale) {
      globalInformation.basicrendercanvas.width = globalInformation.vpblocks.x * globalInformation.renderscale;
      globalInformation.basicrendercanvas.height = globalInformation.vpblocks.y * globalInformation.renderscale;
    }
    basicrendercanvasctx.setTransform(1, 0, 0, 1, 0, 0);;
    basicrendercanvasctx.fillStyle = 'rgb(213,233,249)';
    basicrendercanvasctx.fillRect(0, 0, globalInformation.basicrendercanvas.width, globalInformation.basicrendercanvas.height);



    basicrendercanvasctx.translate(globalInformation.vpblocks.x / 2 * globalInformation.renderscale, globalInformation.vpblocks.y / 2 * globalInformation.renderscale);
    basicrendercanvasctx.scale(globalInformation.renderscale, -globalInformation.renderscale); {
      basicrendercanvasctx.save();

      var cxchunk = Math.floor(globalInformation.cam.x / 10);
      var cychunk = Math.floor(globalInformation.cam.y / 10);

      for (var ychunk = cychunk - globalInformation.vpchunks.y; ychunk <= cychunk + globalInformation.vpchunks.y; ychunk++) {
        for (var xchunk = cxchunk - globalInformation.vpchunks.x; xchunk <= cxchunk + globalInformation.vpchunks.x; xchunk++) {
          if (xchunk < 0 || ychunk < 0 || xchunk >= globalInformation.worldsizechunks.x || ychunk >= globalInformation.worldsizechunks.y) continue;
          var thischunk = worldchunkdata[ychunk][xchunk]
          if (thischunk) {
            basicrendercanvasctx.lineWidth = .03;

            for (var i = 0; i < thischunk.length; i++) { //Draw filled in shapes
              if (!thischunk[i].isWhole) continue;
              if (thischunk[i].materialType == 0) {
                basicrendercanvasctx.fillStyle = "#00ff44";
              } else if (thischunk[i].materialType == 1) {
                basicrendercanvasctx.fillStyle = "#804d0f";
              } else if (thischunk[i].materialType == 2) {
                basicrendercanvasctx.fillStyle = "#696969";
              }
              basicrendercanvasctx.beginPath();
              //console.log(thischunk[i]);

              basicrendercanvasctx.moveTo(thischunk[i].asVec[0] - globalInformation.cam.x, thischunk[i].asVec[1] - globalInformation.cam.y);
              for (var j = 0; j < thischunk[i].asVec.length; j += 2) {
                basicrendercanvasctx.lineTo(thischunk[i].asVec[j] - globalInformation.cam.x, thischunk[i].asVec[j + 1] - globalInformation.cam.y);
                //console.log(thischunk[i].asVec.length);
              }
              basicrendercanvasctx.closePath();
              basicrendercanvasctx.fill();
            }
            for (var i = 0; i < thischunk.length; i++) { //Erase holes
              if (thischunk[i].isWhole) continue;
              basicrendercanvasctx.fillStyle = "#ffffff";

              basicrendercanvasctx.beginPath();
              //console.log(thischunk[i]);

              basicrendercanvasctx.moveTo(thischunk[i].asVec[0] - globalInformation.cam.x, thischunk[i].asVec[1] - globalInformation.cam.y);
              for (var j = 0; j < thischunk[i].asVec.length; j += 2) {
                basicrendercanvasctx.lineTo(thischunk[i].asVec[j] - globalInformation.cam.x, thischunk[i].asVec[j + 1] - globalInformation.cam.y);
                //console.log(thischunk[i].asVec.length);
              }
              basicrendercanvasctx.closePath();
              basicrendercanvasctx.fill();
            }
          }

        }
      }
      basicrendercanvasctx.restore();
    }
    basicrendercanvasctx.translate(-globalInformation.cam.x, -globalInformation.cam.y);
    worldblockdata.forEachFrom(globalInformation.cam.x - globalInformation.vpchunks.x * 10, globalInformation.cam.y - globalInformation.vpchunks.y * 10, globalInformation.cam.x + globalInformation.vpchunks.x * 10, globalInformation.cam.y + globalInformation.vpchunks.y * 10, function(type, x, y) {
      basicrendercanvasctx.translate(x, y);

      switch (type) {
        //NULLBLOCK, STONEBLOCK, PLANKBLOCK, STONEBRICKBLOCK, LOGVBLOCK, LOGHBLOCK, GLASSBLOCK, TORCHBLOCK, LEAFBLOCK
        case globalInformation.blockType.NULLBLOCK:
          break;
        case globalInformation.blockType.STONEBLOCK:
          basicrendercanvasctx.fillStyle = 'gray';
          basicrendercanvasctx.fillRect(0, 0, 1, 1);
          break;
        case globalInformation.blockType.PLANKBLOCK:
          basicrendercanvasctx.fillStyle = 'rgb(159, 105, 53)';
          basicrendercanvasctx.fillRect(0, 0, 1, 1);
          var thisoffset = ((x * 5325423 + y * 124124124) % 634543) / 10575.7167;
          basicrendercanvasctx.fillStyle = `rgb(${237+thisoffset}, ${192+thisoffset}, ${114+thisoffset})`;
          basicrendercanvasctx.fillRect(0, 0, 1, 1 / 3);

          thisoffset = ((x * 23423423 + y * 214214) % 634543) / 10575.7167;
          basicrendercanvasctx.fillStyle = `rgb(${237+thisoffset}, ${192+thisoffset}, ${114+thisoffset})`;
          basicrendercanvasctx.fillRect(0, 1 / 3, 1 / 2, 1 / 3);
          thisoffset = (((1 + x) * 23423423 + y * 214214) % 634543) / 10575.7167;
          basicrendercanvasctx.fillStyle = `rgb(${237+thisoffset}, ${192+thisoffset}, ${114+thisoffset})`;
          basicrendercanvasctx.fillRect(1 / 2, 1 / 3, 1 / 2, 1 / 3);

          thisoffset = (((x) * 53252351 + y * 52353) % 634543) / 10575.7167;
          basicrendercanvasctx.fillStyle = `rgb(${237+thisoffset}, ${192+thisoffset}, ${114+thisoffset})`;
          basicrendercanvasctx.fillRect(0, 2 / 3, 1, 1 / 3);
          break;
        case globalInformation.blockType.STONEBRICKBLOCK:
          basicrendercanvasctx.fillStyle = 'rgb(158, 158, 158)';
          basicrendercanvasctx.fillRect(0, 0, 1, 1);
          basicrendercanvasctx.fillStyle = 'rgb(122, 122, 122)';
          let devamt = .04
          basicrendercanvasctx.fillRect(0 + devamt, 0, 1 - 2 * devamt, 1 / 2 - devamt);
          basicrendercanvasctx.fillRect(0, 1 / 2 + devamt, 1 / 2 - devamt, 1 / 2 - devamt);
          basicrendercanvasctx.fillRect(1 / 2 + devamt, 1 / 2 + devamt, 1 / 2 - devamt, 1 / 2 - devamt);
          break;
        case globalInformation.blockType.LOGVBLOCK:
          basicrendercanvasctx.fillStyle = 'black';
          basicrendercanvasctx.fillRect(0, 0, 1, 1);
          for (var yy = 0; yy < 10; yy++) {
            for (var xx = 0; xx < 10; xx++) {
              thisoffseta = ((x * 53252351 + y * 52323555223 + xx * 1242234253224 + yy * 62554) % 123533) / 5575.7167;
              thisoffsetb = ((x * 235325 + y * 26342355234 + xx * 64353453 + yy * 62554) % 1253235) / 70575.7167;
              thisoffsetc = thisoffsetb * 1.5;
              thisoffseta += ((x * 235325 + y * 26342355234 + xx * 64353453 + yy * 62554) % 1253235) / 40575.7167;
              thisoffsetb += ((x * 235325 + y * 26342355234 + xx * 64353453 + yy * 62554) % 1253235) / 40575.7167;
              thisoffsetc += ((x * 235325 + y * 26342355234 + xx * 64353453 + yy * 62554) % 1253235) / 40575.7167;


              basicrendercanvasctx.fillStyle = `rgb(${125+thisoffseta}, ${87+thisoffsetb}, ${2+thisoffsetc})`;
              basicrendercanvasctx.fillRect(xx / 10, yy / 10, 1 / 10 + (xx == 9 ? 0 : 1 / 15), 1 / 10 + (yy == 9 ? 0 : 1 / 15));
            }
          }
          break;
        case globalInformation.blockType.LOGHBLOCK:
          for (var yy = 0; yy < 10; yy++) {
            for (var xx = 0; xx < 10; xx++) {
              thisoffseta = ((x * 53252351 + y * 52323555223 + yy * 1242234253224 + xx * 62554) % 123533) / 5575.7167;
              thisoffsetb = ((x * 235325 + y * 26342355234 + yy * 64353453 + xx * 62554) % 1253235) / 70575.7167;
              thisoffsetc = thisoffsetb * 1.5;
              thisoffseta += ((x * 235325 + y * 26342355234 + yy * 64353453 + xx * 62554) % 1253235) / 40575.7167;
              thisoffsetb += ((x * 235325 + y * 26342355234 + yy * 64353453 + xx * 62554) % 1253235) / 40575.7167;
              thisoffsetc += ((x * 235325 + y * 26342355234 + yy * 64353453 + xx * 62554) % 1253235) / 40575.7167;


              basicrendercanvasctx.fillStyle = `rgb(${125+thisoffseta}, ${87+thisoffsetb}, ${2+thisoffsetc})`;
              basicrendercanvasctx.fillRect(xx / 10, yy / 10, 1 / 10 + (xx == 9 ? 0 : 1 / 15), 1 / 10 + (yy == 9 ? 0 : 1 / 15));
            }
          }
          break;


        case globalInformation.blockType.GLASSBLOCK:
          basicrendercanvasctx.fillStyle = 'rgba(250,250,250,.4)';
          basicrendercanvasctx.fillRect(0, 0, 1, 1);
          break;
        case globalInformation.blockType.TORCHBLOCK:
          basicrendercanvasctx.fillStyle = 'rgb(125, 87, 2)';
          basicrendercanvasctx.fillRect(.4, 0, .2, .6);
          basicrendercanvasctx.fillStyle = 'rgba(255, 218, 118,.6)';
          basicrendercanvasctx.beginPath();
          basicrendercanvasctx.arc(.5, .7, .3, 0, 2 * Math.PI, false);
          basicrendercanvasctx.fill();
          basicrendercanvasctx.fillStyle = 'rgba(255, 248, 148,.6)';
          basicrendercanvasctx.beginPath();
          basicrendercanvasctx.arc(.5, .7, .2, 0, 2 * Math.PI, false);
          basicrendercanvasctx.fill();
          break;
        case globalInformation.blockType.LEAFBLOCK:
          basicrendercanvasctx.fillStyle = 'black';
          basicrendercanvasctx.fillRect(0, 0, 1, 1);
          for (var yy = 0; yy < 10; yy++) {
            for (var xx = 0; xx < 10; xx++) {
              thisoffseta = ((x * 352532 + y * 53253 + yy * 235235 + xx * 235353) % 2352332) / 55075.7167;
              thisoffsetb = ((x * 235325 + y * 3252353 + yy * 64353453 + xx * 23523523) % 1253235) / 55705.7167;
              thisoffsetc = ((x * 23532 + y * 32523523 + yy * 64353453 + xx * 5325) % 1253235) / 55075.7167;



              basicrendercanvasctx.fillStyle = `rgb(${0+thisoffseta}, ${150+thisoffsetb}, ${24+thisoffsetc})`;
              basicrendercanvasctx.fillRect(xx / 10, yy / 10, 1 / 10 + (xx == 9 ? 0 : 1 / 15), 1 / 10 + (yy == 9 ? 0 : 1 / 15));
            }
          }
          break;

        default:
          basicrendercanvasctx.fillStyle = 'pink';
          basicrendercanvasctx.fillRect(0, 0, .5, .5);
          basicrendercanvasctx.fillRect(.5, .5, .5, .5);
          basicrendercanvasctx.fillStyle = 'black';
          basicrendercanvasctx.fillRect(.5, 0, .5, .5);
          basicrendercanvasctx.fillRect(0, .5, .5, .5);

      }
      basicrendercanvasctx.translate(-x, -y);
    })

    for (var i = 0; i < globalInformation.renderlist.length; i++) {
      basicrendercanvasctx.save();
      globalInformation.renderlist[i](basicrendercanvasctx);
      basicrendercanvasctx.restore();
    }

    {
      var itemsize = globalInformation.renderscale * 1.2;

      var itemcount = 0;
      new Array(Object.keys(globalInformation.itemType).length).fill(0).map((a, b) => ("ITM" + b)).map((a, b) => {
        if (globalInformation.thisplayer[a]) itemcount++
      })
      var thisitemtype = 0;
      //console.log(itemcount);
      Object.keys(globalInformation.itemType).length;

      basicrendercanvasctx.setTransform(1, 0, 0, 1, 0, 0);;
      basicrendercanvasctx.fillStyle = 'rgba(250,250,250,.4)';
      basicrendercanvasctx.strokeStyle = 'rgba(250,250,250,.7)';
      basicrendercanvasctx.lineWidth = globalInformation.renderscale * .05;

      basicrendercanvasctx.fillRect(globalInformation.basicrendercanvas.width / 2 - itemsize * itemcount / 2, globalInformation.basicrendercanvas.height - itemsize, itemsize * itemcount, itemsize);
      basicrendercanvasctx.strokeRect(globalInformation.basicrendercanvas.width / 2 - itemsize * itemcount / 2, globalInformation.basicrendercanvas.height - itemsize, itemsize * itemcount, itemsize + 10);
      for (var i = 0; i < itemcount; i++) {
        while (globalInformation.thisplayer["ITM" + thisitemtype] == 0 && thisitemtype < Object.keys(globalInformation.itemType).length) thisitemtype++;
        //console.log(thisitemtype);
        var xc = globalInformation.basicrendercanvas.width / 2 - itemsize * itemcount / 2 + itemsize / 2 + itemsize * i;
        var yc = globalInformation.basicrendercanvas.height - itemsize / 2;
        basicrendercanvasctx.translate(xc, yc);
        basicrendercanvasctx.scale(globalInformation.renderscale, globalInformation.renderscale);
        if (renderItemInv[thisitemtype]) {
          renderItemInv[thisitemtype](basicrendercanvasctx);
        } else {
          basicrendercanvasctx.fillStyle = 'pink';
          basicrendercanvasctx.fillRect(0 - .5, 0 - .5, .5, .5);
          basicrendercanvasctx.fillRect(.5 - .5, .5 - .5, .5, .5);
          basicrendercanvasctx.fillStyle = 'black';
          basicrendercanvasctx.fillRect(.5 - .5, 0 - .5, .5, .5);
          basicrendercanvasctx.fillRect(0 - .5, .5 - .5, .5, .5);
        }

        basicrendercanvasctx.scale(1 / globalInformation.renderscale, 1 / globalInformation.renderscale);
        basicrendercanvasctx.font = (.5 * globalInformation.renderscale) + "px Arial";
        basicrendercanvasctx.fillStyle = 'rgb(80, 80, 80)';
        //console.log(globalInformation.thisplayer)
        basicrendercanvasctx.fillText(globalInformation.thisplayer["ITM" + thisitemtype], (.2) * globalInformation.renderscale, (.4) * globalInformation.renderscale);
        basicrendercanvasctx.translate(-xc, -yc);
        thisitemtype++;
      }
    }
  }*/

  function calculateSkylight(cxchunk, cychunk) {
    var skylighthere = new Uint32Array(10 * (globalInformation.vpchunks.x * 2 + 1));
    for (var ychunk = cychunk + globalInformation.vpchunks.y + 1; ychunk < globalInformation.worldsizechunks.y; ychunk++) {
      for (var xchunk = cxchunk - globalInformation.vpchunks.x; xchunk <= cxchunk + globalInformation.vpchunks.x; xchunk++) {
        var thisskylight = worldskylightdata[ychunk][xchunk];
        if (thisskylight) {
          for (var i = 0; i < 10; i++) {
            skylighthere[10 * (xchunk - cxchunk + globalInformation.vpchunks.x) + i] |= thisskylight[i];
          }
        }
      }
    }
    //                bytesPerPixel  blocksPerChunk                 chunksInViewportX  bytesPerBlock
    let dat = new Uint8ClampedArray(4 * 10 * (globalInformation.vpchunks.x * 2 + 1) * 32);
    for (let i = 0; i < 10 * (globalInformation.vpchunks.x * 2 + 1); i++) {
      for (let j = 0; j < 32; j++) {
        dat[(i * 32 + j) * 4] = (skylighthere[i] & (1 << (31 - j))) === 0 ? 255 : 0;
        dat[(i * 32 + j) * 4 + 3] = 255;
      }
    }

    let canvas = globalInformation.skylightdatacanvas,
      ctx = canvas.getContext('2d');
    if (canvas.width !== 32 * (globalInformation.vpblocks.x) || canvas.height !== 1) {
      canvas.width = 32 * (globalInformation.vpblocks.x);
      canvas.height = 1;
    }
    var imgDat = ctx.createImageData(dat.length / 4, canvas.height);
    imgDat.data.set(dat);
    ctx.putImageData(imgDat, 32 * ((cxchunk - globalInformation.vpchunks.x) * 10 - (globalInformation.vpblocks.x * -.5 + globalInformation.cam.x)), 0);

  }

  function fancyrender() {
    rendermateriallayers(); {
      function setup(canvas) {
        var ctx = canvas.getContext('2d');
        ctx.setTransform(1, 0, 0, 1, 0, 0);;
        ctx.translate(globalInformation.vpblocks.x / 2 * globalInformation.renderscale, globalInformation.vpblocks.y / 2 * globalInformation.renderscale);
        ctx.scale(globalInformation.renderscale, -globalInformation.renderscale);
        return ctx;
      }

      var foregroundrendercanvasctx = setup(globalInformation.foregroundrendercanvas);
      foregroundrendercanvasctx.translate(-globalInformation.cam.x, -globalInformation.cam.y);

      for (var i = 0; i < globalInformation.renderlist.length; i++) {
        foregroundrendercanvasctx.save();
        globalInformation.renderlist[i](foregroundrendercanvasctx);
        foregroundrendercanvasctx.restore();
      }
    }
    var cxchunk = Math.floor(globalInformation.cam.x / 10);
    var cychunk = Math.floor(globalInformation.cam.y / 10);

    calculateSkylight(cxchunk, cychunk);

    resetLineBuffer();

    function translateToGLCoordsX(x) {
      return ((x - globalInformation.cam.x) / globalInformation.vpblocks.x + .5)
    }

    function translateToGLCoordsY(y) {
      return ((y - globalInformation.cam.y) / globalInformation.vpblocks.y + .5)
    }
    for (var ychunk = cychunk - globalInformation.vpchunks.y; ychunk <= cychunk + globalInformation.vpchunks.y; ychunk++) {
      for (var xchunk = cxchunk - globalInformation.vpchunks.x; xchunk <= cxchunk + globalInformation.vpchunks.x; xchunk++) {
        if (xchunk < 0 || ychunk < 0 || xchunk >= globalInformation.worldsizechunks.x || ychunk >= globalInformation.worldsizechunks.y) continue;

        var thischunk = worldchunkdata[ychunk][xchunk]
        if (thischunk) {
          for (var i = 0; i < thischunk.length; i++) {
            for (var j = 0; j < thischunk[i].asVec.length - 2; j += 2) {
              addLineToBuffer(translateToGLCoordsX(thischunk[i].asVec[j]),
                translateToGLCoordsY(thischunk[i].asVec[j + 1]),
                translateToGLCoordsX(thischunk[i].asVec[j + 2]),
                translateToGLCoordsY(thischunk[i].asVec[j + 3]));
            }
            addLineToBuffer(translateToGLCoordsX(thischunk[i].asVec[0]),
              translateToGLCoordsY(thischunk[i].asVec[1]),
              translateToGLCoordsX(thischunk[i].asVec[thischunk[i].asVec.length - 2]),
              translateToGLCoordsY(thischunk[i].asVec[thischunk[i].asVec.length - 1]));
          }
        }
      }
    }


    var k = 0;
    worldblockdata.forEachFrom(10 * (cxchunk - globalInformation.vpchunks.x),
      10 * (cychunk - globalInformation.vpchunks.y),
      10 * (cxchunk + globalInformation.vpchunks.x + 1),
      10 * (cychunk + globalInformation.vpchunks.y + 1), (o, x, y) => {
        addLineToBuffer(translateToGLCoordsX(x), translateToGLCoordsY(y), translateToGLCoordsX(x + 1), translateToGLCoordsY(y));
        addLineToBuffer(translateToGLCoordsX(x), translateToGLCoordsY(y + 1), translateToGLCoordsX(x + 1), translateToGLCoordsY(y + 1));
        addLineToBuffer(translateToGLCoordsX(x), translateToGLCoordsY(y), translateToGLCoordsX(x), translateToGLCoordsY(y + 1));
        addLineToBuffer(translateToGLCoordsX(x + 1), translateToGLCoordsY(y), translateToGLCoordsX(x + 1), translateToGLCoordsY(y + 1));
      });

    for (var i = 0; i < globalInformation.rendershadelist.length; i += 4) {
      addLineToBuffer(translateToGLCoordsX(globalInformation.rendershadelist[i]), translateToGLCoordsY(globalInformation.rendershadelist[i + 1]), translateToGLCoordsX(globalInformation.rendershadelist[i + 2]), translateToGLCoordsY(globalInformation.rendershadelist[i + 3]));
    }

    resetLightBuffer();
    for (var i = 0; i < globalInformation.renderslightlist.length; i += 5) {
      //addLightToBuffer(translateToGLCoordsX(globalInformation.renderslightlist[i]), translateToGLCoordsY(globalInformation.renderslightlist[i + 1]), globalInformation.renderslightlist[i + 2], globalInformation.renderslightlist[i + 3], globalInformation.renderslightlist[i + 4]);
    }

    for (var x = 0; x < 1; x += .05) {
      //addLightToBuffer(x,1,.5-Math.abs(x-.25),.5-Math.abs(x-.5),.5-Math.abs(x-.75));
    }

    //addLightToBuffer(translateToGLCoordsX(globalInformation.thisplayer.x),translateToGLCoordsY(globalInformation.thisplayer.y),.5,.5,.5);

    //lightcolor*.5/(pow(1000/3.5,1.5)+.4)
    {
      addLightToBuffer(globalInformation.shadeMarkAsSkylight, 0, .4, .55,.7)
    }
    addLightToBuffer(globalInformation.debug.mp2.x, globalInformation.debug.mp2.y,.5, .5, .5);

    renderShadeCanvas();
    renderCombinedCanvas();

  }
  //camera 20 by 12

  function binUpTo(percent) {
    return (((-1 << ((32 * (1 - percent)) >>> 0) >>> 0)));
    if (percent < 1 / 32) return (0 >>> 0);
  }

  function binFrom(percent) {
    if (percent >= 1) return (0 >>> 0);
    return ((-1 >>> (32 * (percent))));
  }

  var ws = new WebSocket("ws://localhost:9002");

  ws.onopen = function(event) {
    console.log("opened");
  };
  ws.onmessage = function(event) {

    (new Response(event.data).arrayBuffer()).then(function(x) {

      var ds = deserializer.deserialize(x,
        [
          [
            ["x", deserializer.type.FLOAT],
            ["y", deserializer.type.FLOAT],
            ...new Array(Object.keys(globalInformation.itemType).length).fill(0).map((a, b) => (["ITM" + b, deserializer.type.UINT]))
          ],
          [
            ["x", deserializer.type.FLOAT],
            ["y", deserializer.type.FLOAT],
            ["xv", deserializer.type.FLOAT],
            ["yv", deserializer.type.FLOAT],
            ["rot", deserializer.type.FLOAT],
            ["health", deserializer.type.FLOAT],
            ["name", deserializer.type.STRING]
          ],
          [
            ["x", deserializer.type.FLOAT],
            ["y", deserializer.type.FLOAT],
            ["rot", deserializer.type.FLOAT],
            ["itemtype", deserializer.type.UINT],
            ["count", deserializer.type.UINT],
          ],
          [
            ["x", deserializer.type.FLOAT],
            ["y", deserializer.type.FLOAT],
            ["rot", deserializer.type.FLOAT],
          ],
          [
            ["x", deserializer.type.FLOAT],
            ["y", deserializer.type.FLOAT],
            ["potiontype", deserializer.type.UINT],
          ],

          [
            ["isWhole", deserializer.type.BYTE],
            ["materialType", deserializer.type.BYTE],
            ["asVec", deserializer.type.XYPAIRS],
            ["chunkx", deserializer.type.UINT],
            ["chunky", deserializer.type.UINT],
            ["isBackground", deserializer.type.BYTE],

          ],
          [
            ["x", deserializer.type.INTR],
            ["y", deserializer.type.INTR],
            ["thisBlockType", deserializer.type.BYTE],
          ],

        ]
      );
      var fds = {
        clientThisPlayers: ds[0],
        players: ds[1],
        items: ds[2],
        bullets: ds[3],
        potions: ds[4],
        blocks: ds[6],
        clientChunkPieces: ds[5],

      }

      //ThisPLayer
      globalInformation.thisplayer = fds.clientThisPlayers[0];
      {
        var preserve = .1;
        globalInformation.cam.x = globalInformation.cam.x * preserve + globalInformation.thisplayer.x * (1 - preserve);
        globalInformation.cam.y = globalInformation.cam.y * preserve + globalInformation.thisplayer.y * (1 - preserve);

      }

      //Renderlist
      globalInformation.renderlist = [];
      globalInformation.rendershadelist = [];
      globalInformation.renderslightlist = [];


      for (var i = 0; i < fds.players.length; i++) {
        function mult(a, b) {
          return ({
            x: a.x * b.x - a.y * b.y,
            y: a.x * b.y + a.y * b.x
          });
        }

        {
          let plr = fds.players[i];

          let renderpoints = computePARenderPoints({
            x: plr.x,
            xv: plr.xv,
            yv: plr.yv
          });

          let ang = plr.rot;
          let rotVec = {
            x: Math.cos(ang) / 3 * 2,
            y: Math.sin(ang) * .8
          };
          let translate={
            x:plr.x,
            y:plr.y,
          };
          function drawLine(x1, y1, x2, y2) {
            var a = mult({
              x: x1,
              y: y1
            }, rotVec);
            var b = mult({
              x: x2,
              y: y2
            }, rotVec);
            globalInformation.rendershadelist.push(a.x+translate.x, a.y+translate.y,b.x+translate.x, b.y+translate.y);
          }
          drawLine(renderpoints.neck.x, renderpoints.neck.y, renderpoints.hips.x, renderpoints.hips.y);
          drawLine(renderpoints.neck.x, renderpoints.neck.y, renderpoints.rightElbow.x, renderpoints.rightElbow.y);
          drawLine(renderpoints.neck.x, renderpoints.neck.y, renderpoints.leftElbow.x, renderpoints.leftElbow.y);
          drawLine(renderpoints.rightKnee.x, renderpoints.rightKnee.y, renderpoints.hips.x, renderpoints.hips.y);
          drawLine(renderpoints.leftKnee.x, renderpoints.leftKnee.y, renderpoints.hips.x, renderpoints.hips.y);
          drawLine(renderpoints.rightKnee.x, renderpoints.rightKnee.y, renderpoints.rightFoot.x, renderpoints.rightFoot.y);
          drawLine(renderpoints.leftKnee.x, renderpoints.leftKnee.y, renderpoints.leftFoot.x, renderpoints.leftFoot.y);
          drawLine(renderpoints.rightElbow.x, renderpoints.rightElbow.y, renderpoints.rightHand.x, renderpoints.rightHand.y);
          drawLine(renderpoints.leftElbow.x, renderpoints.leftElbow.y, renderpoints.leftHand.x, renderpoints.leftHand.y);
          var hp = mult(renderpoints.head, rotVec);

          for(let r=0;r<5;r++){
            let rot1=r/5*2*Math.PI;
            let rot2=(r+1)/5*2*Math.PI;
            let ax=Math.cos(rot1)*.26+hp.x+translate.x;
            let ay=Math.sin(rot1)*.26+hp.y+translate.y;

            let bx=Math.cos(rot2)*.26+hp.x+translate.x;
            let by=Math.sin(rot2)*.26+hp.y+translate.y;
            globalInformation.rendershadelist.push(ax, ay,bx, by);

          }
        }
        {
          globalInformation.renderlist.push(function() {
            var plr = fds.players[i];

            return (function(ctx) {
              let renderpoints = computePARenderPoints({
                x: plr.x,
                xv: plr.xv,
                yv: plr.yv
              });

              var ang = plr.rot;
              var rotVec = {
                x: Math.cos(ang) / 3 * 2,
                y: Math.sin(ang) * .8
              };


              function drawLine(x1, y1, x2, y2) {
                ctx.beginPath();
                var a = mult({
                  x: x1,
                  y: y1
                }, rotVec);
                ctx.moveTo(a.x, a.y);
                var b = mult({
                  x: x2,
                  y: y2
                }, rotVec);
                ctx.lineTo(b.x, b.y);
                ctx.stroke();
              }
              ctx.lineWidth = .05;
              ctx.strokeStyle = "#FF0000";

              ctx.translate(plr.x, plr.y);

              drawLine(renderpoints.neck.x, renderpoints.neck.y, renderpoints.hips.x, renderpoints.hips.y);
              drawLine(renderpoints.neck.x, renderpoints.neck.y, renderpoints.rightElbow.x, renderpoints.rightElbow.y);
              drawLine(renderpoints.neck.x, renderpoints.neck.y, renderpoints.leftElbow.x, renderpoints.leftElbow.y);
              drawLine(renderpoints.rightKnee.x, renderpoints.rightKnee.y, renderpoints.hips.x, renderpoints.hips.y);
              drawLine(renderpoints.leftKnee.x, renderpoints.leftKnee.y, renderpoints.hips.x, renderpoints.hips.y);
              drawLine(renderpoints.rightKnee.x, renderpoints.rightKnee.y, renderpoints.rightFoot.x, renderpoints.rightFoot.y);
              drawLine(renderpoints.leftKnee.x, renderpoints.leftKnee.y, renderpoints.leftFoot.x, renderpoints.leftFoot.y);
              drawLine(renderpoints.rightElbow.x, renderpoints.rightElbow.y, renderpoints.rightHand.x, renderpoints.rightHand.y);
              drawLine(renderpoints.leftElbow.x, renderpoints.leftElbow.y, renderpoints.leftHand.x, renderpoints.leftHand.y);
              var hp = mult(renderpoints.head, rotVec)
              ctx.beginPath();
              ctx.arc(hp.x, hp.y, .26, 0, 2 * Math.PI);
              ctx.stroke();

              //var mx=mousepos.x/globalInformation.renderscale-globalInformation.vpblocks.x*.5,my=globalInformation.vpblocks.y*.5-mousepos.y/globalInformation.renderscale;
              //var mp=computeMiddlePoint({x:mx,y:my},{x:0,y:0},2);
            });
          }());
        }
      }


      for (var i = 0; i < fds.items.length; i++) {
        globalInformation.renderlist.push(function() {
          var tp = fds.items[i];
          return (function(ctx) {

            ctx.translate(tp.x, tp.y);
            ctx.fillStyle = 'black';

            ctx.rotate(tp.rot)
            ctx.fillRect(-.5 / 2, -.1 / 2, .5, .1);
          });

        }()); {
          var itm = fds.items[i];

          let len = Math.hypot((-.1 / 2), (-.5 / 2)),
            dir = Math.atan((-.1 / 2) / (-.5 / 2)),
            x1 = itm.x + len * Math.cos(itm.rot + dir),
            y1 = itm.y + len * Math.sin(itm.rot + dir),
            x2 = itm.x + len * Math.cos(itm.rot + (Math.PI - dir)),
            y2 = itm.y + len * Math.sin(itm.rot + (Math.PI - dir)),
            x3 = itm.x + len * Math.cos(itm.rot + -(Math.PI - dir)),
            y3 = itm.y + len * Math.sin(itm.rot + -(Math.PI - dir));
          x4 = itm.x + len * Math.cos(itm.rot + -dir), y4 = itm.y + len * Math.sin(itm.rot + -dir),
            globalInformation.rendershadelist.push(x1, y1, x2, y2);
          globalInformation.rendershadelist.push(x2, y2, x3, y3);
          globalInformation.rendershadelist.push(x3, y3, x4, y4);
          globalInformation.rendershadelist.push(x4, y4, x1, y1);
          //globalInformation.renderslightlist.push(itm.x,itm.y,1,.5,.5)
        }
      }


      var blocklocsBlockcanvas = [];
      var blockdataBlockcanvas = [];


      for (var i = 0; i < fds.blocks.length; i++) {
        //console.log("blockinfo");
        var tp = fds.blocks[i];

        blocklocsBlockcanvas.push(tp.x, tp.y);
        blockdataBlockcanvas.push(tp.thisBlockType);

        if (tp.thisBlockType === 0) {
          worldblockdata.delete(tp.x, tp.y);
        } else {
          worldblockdata.put(tp.x, tp.y, tp.thisBlockType);
        }
      }



      setBlockCanvas(blocklocsBlockcanvas, blockdataBlockcanvas);


      //Chunk
      var hasbeentouched = {};
      var hasbeentouchedbg = {};

      for (var i = 0; i < fds.clientChunkPieces.length; i++) {
        var dc = fds.clientChunkPieces[i];
        if (dc.isBackground) {
          if (!hasbeentouchedbg[dc.chunkx + "|" + dc.chunky]) { //quick fix
            worldbackgroundchunkdata[dc.chunky][dc.chunkx] = [];
          }
          hasbeentouchedbg[dc.chunkx + "|" + dc.chunky] = true;
          worldbackgroundchunkdata[dc.chunky][dc.chunkx].push(dc);
        } else {
          if (!hasbeentouched[dc.chunkx + "|" + dc.chunky]) { //quick fix
            worldchunkdata[dc.chunky][dc.chunkx] = [];
            worldskylightdata[dc.chunky][dc.chunkx] = new Uint32Array(10);
          }
          hasbeentouched[dc.chunkx + "|" + dc.chunky] = true;
          worldchunkdata[dc.chunky][dc.chunkx].push(dc);

          {
            var asVec = dc.asVec;
            for (var j = 0; j < asVec.length - 2; j += 2) {
              var xstart = asVec[j + 0] - dc.chunkx * 10;
              var xend = asVec[j + 2] - dc.chunkx * 10;
              var xmin = Math.min(xstart, xend);
              var xmax = Math.max(xstart, xend);
              for (var x = Math.floor(xmin); x < Math.ceil(xmax); x++) {
                if (x == Math.floor(xmin) || x == Math.ceil(xmax) - 1) {

                  if (x == Math.floor(xmin) && x == Math.ceil(xmax) - 1) {
                    worldskylightdata[dc.chunky][dc.chunkx][x] = (worldskylightdata[dc.chunky][dc.chunkx][x]) | (binUpTo(xmax - x) & binFrom(xmin - x));
                  } else if (x == Math.floor(xmin)) {
                    worldskylightdata[dc.chunky][dc.chunkx][x] = (worldskylightdata[dc.chunky][dc.chunkx][x]) | binFrom(xmin - x);
                  } else {
                    worldskylightdata[dc.chunky][dc.chunkx][x] = (worldskylightdata[dc.chunky][dc.chunkx][x]) | binUpTo(xmax - x);
                  }
                } else {
                  worldskylightdata[dc.chunky][dc.chunkx][x] = -1; //111...1
                }
              }
            }
          }
        }
      }


    });



  }
  window.addEventListener("keydown", function() {
    var x = event.which || event.keyCode;
    var succesful = false;
    if (x == 87 || x == 38) {
      succesful = true;
      movementkeyboard[0] = true; //W
    }
    if (x == 65 || x == 37) {
      succesful = true;
      movementkeyboard[1] = true; //A
    }
    if (x == 83 || x == 40) {
      succesful = true;
      movementkeyboard[2] = true; //S
    }
    if (x == 68 || x == 39) {
      succesful = true;
      movementkeyboard[3] = true; //D
    }
    if (succesful) event.preventDefault();
  });
  window.addEventListener("keyup", function() {
    var x = event.which || event.keyCode;
    var succesful = false;
    if (x == 87 || x == 38) {
      succesful = true;
      movementkeyboard[0] = false; //W
    }
    if (x == 65 || x == 37) {
      succesful = true;
      movementkeyboard[1] = false; //A
    }
    if (x == 83 || x == 40) {
      succesful = true;
      movementkeyboard[2] = false; //S
    }
    if (x == 68 || x == 39) {
      succesful = true;
      movementkeyboard[3] = false; //D
    }
    if (succesful) event.preventDefault();
  });
  window.addEventListener("mousedown", function() {
    mousedown = true;
  });
  window.addEventListener("mouseup", function() {
    mousedown = false;
  });
  window.addEventListener("mousemove", function(evt) {
    var rect = globalInformation.combinedshadercanvas.getBoundingClientRect();
    mousepos.x = evt.clientX - rect.left;
    mousepos.y = evt.clientY - rect.top;

    var mp2 = {};
    //rect = globalInformation.shadecanvas.getBoundingClientRect();
    mp2.x = evt.clientX - rect.left;
    mp2.y = evt.clientY - rect.top;
    mp2.x /= rect.width;
    mp2.y /= rect.height;
    mp2.y = 1 - mp2.y;
    globalInformation.debug.mp2 = mp2;
    //mousepos.x=event;
  });

  var doIO = function() {
    var buffer = new ArrayBuffer(13);
    var view = new DataView(buffer);
    view.setUint8(0, movementkeyboard[0] * 1);
    view.setUint8(1, movementkeyboard[1] * 1);
    view.setUint8(2, movementkeyboard[2] * 1);
    view.setUint8(3, movementkeyboard[3] * 1);
    var mpxb = (mousepos.x - globalInformation.vpblocks.x / 2 * globalInformation.renderscale) / globalInformation.renderscale + globalInformation.cam.x;
    var mpyb = -(mousepos.y - globalInformation.vpblocks.y / 2 * globalInformation.renderscale) / globalInformation.renderscale + globalInformation.cam.y;
    view.setFloat32(4, mpxb)
    view.setFloat32(8, mpyb)
    view.setUint8(12, mousedown * 1);
    ws.send(buffer);
  }

  function frame() {
    var t0 = performance.now();
    try {
      //basicrender();
      fancyrender();
      doIO();
      globalInformation.currentframecount++;

    } catch (e) {
      console.error(e);
    }
    var t1 = performance.now();
    setTimeout(frame, (1000 / globalInformation.framespersec) - (t1 - t0));


  }
  setInterval(() => {
    globalInformation.realframepersec = globalInformation.currentframecount;
    console.log("FPS: " + globalInformation.realframepersec);
    globalInformation.currentframecount = 0;
  }, 1000);
  {
    let maxX=26.6,
        maxY=15;
    let wX=window.innerWidth,
        wY=window.innerHeight;
    if(maxX/maxY>wX/wY){
      var cX=wX/wY*maxY;
      var cY=maxY;
    }else{
      var cX=maxX;
      var cY=wY/wX*maxX;
    }
    globalInformation.vpblocks.x=cX;
    globalInformation.vpblocks.y=cY;
    globalInformation.renderscale=Math.max(wX/cX,wY/cY);
  }
  webGlINIT();

  frame();

}
