
function computeMiddlePoint(p2,p1,r,flip){
  var dx=p2.x-p1.x;
  var dy=p2.y-p1.y;
  var d=Math.hypot(dx,dy);
  var x=d/2;
  if(r*r<x*x){
    var farestx=-dx/x*r;
    var faresty=-dy/x*r;
    return({joint:{x:farestx/2+p2.x,y:faresty/2+p2.y},far:{x:farestx+p2.x,y:faresty+p2.y}});
  }
  var h=Math.sqrt(r*r-x*x);
  var mx=(p1.x+p2.x)/2;
  var my=(p1.y+p2.y)/2;
  mx+=flip*dy*(h/d);
  my+=-flip*dx*(h/d);
  return({joint:{x:mx,y:my},far:p1});
}
function computePARenderPoints(state){
  const blocksperstep=1.8;
  var legonground=-1;
  function calcOne(posx,rightleg) {
    var rx=(((posx+(rightleg?blocksperstep:0)))%(blocksperstep*2)+(blocksperstep*2))%(blocksperstep*2);
    var amtin=rx/blocksperstep;
    var res;
    var z=.12-Math.max(-.12,Math.min(.12,Math.sin(amtin*Math.PI)*.24));
    if(z===0)legonground=rightleg?1:0;
    //console.log(z);
    res={x:Math.cos(amtin*Math.PI),y:z};

    res.x*=.5;
    return(res);
  }
  var hips={x:0,y:1};
  var neck={x:0,y:2};
  var leftfoottarget=calcOne(state.x,false);
  var rightfoottarget=calcOne(state.x,true);
  if(Math.abs(state.xv)<.5){
    function avg(a,b){
      const rat=1;
      return({x:(a.x*rat+b.x*(1-rat)),y:(a.y*rat+b.y*(1-rat))})
    }
    leftfoottarget=avg({x:-.5,y:-1},leftfoottarget)
    rightfoottarget=avg({x:.5,y:-1},rightfoottarget)

  }
  const leglength=.55;
  const direction=state.xv>0?1:-1;

  var leftfootcalc=computeMiddlePoint(hips,leftfoottarget,leglength,direction);
  var rightfootcalc=computeMiddlePoint(hips,rightfoottarget,leglength,direction);
  if(legonground>=0){
    var ydiff=Math.sqrt(Math.pow(leglength*2,2)-Math.pow((legonground?rightfoottarget:leftfoottarget).x,2))-hips.y;
    hips.y+=ydiff;
    neck.y+=ydiff;

    (legonground?leftfoottarget:rightfoottarget).y+=ydiff;
  }
  leftfootcalc=computeMiddlePoint(hips,leftfoottarget,leglength,direction);
  rightfootcalc=computeMiddlePoint(hips,rightfoottarget,leglength,direction);
  var maxlift=Math.min(rightfootcalc.far.y,leftfootcalc.far.y);
  neck.y-=maxlift;
  hips.y-=maxlift;
  rightfootcalc.joint.y-=maxlift;
  leftfootcalc.joint.y-=maxlift;
  rightfootcalc.far.y-=maxlift;
  leftfootcalc.far.y-=maxlift;
  return({
    neck:neck,
    rightElbow:{x:-.3,y:1.5},
    leftElbow:{x:.3,y:1.5},
    rightHand:{x:-.5,y:1},
    leftHand:{x:.5,y:1},
    hips:hips,
    rightKnee:rightfootcalc.joint,
    leftKnee:leftfootcalc.joint,
    rightFoot:rightfootcalc.far,
    leftFoot:leftfootcalc.far,
    head:{x:neck.x+Math.max(-.1,Math.min(.1,state.xv/5)),y:neck.y+.32}
  });
}
