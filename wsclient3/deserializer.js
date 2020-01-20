const endianness=true;//change to false when moving to linux server

var deserializer={
  type:{
    STRING:1,//n:4 bytes,str: n bytes
    UINT:2,//4 bytes
    FLOAT:3,//4 bytes
    INTR:4,
    BYTE:5,
    XYPAIRS:6,
  },
  deserialize:function (binaryData,classAttrArrays){
    var dv=new DataView(binaryData);
    var deserialized=[];
    var currByteOffset=classAttrArrays.length*4;
    //console.log("start,",binaryData);
    for(var i=0;i<classAttrArrays.length;i++){
      var partLen=dv.getUint32(i*4,endianness);
      //console.log(partLen);

      deserialized.push(this.deserializePart(new DataView(binaryData,currByteOffset,partLen),classAttrArrays[i]));
      currByteOffset+=partLen;
    }
    return(deserialized);
  },
  deserializePart:function(odv,classAttrArray){
    if(odv.byteLength==0){
      return([]);
    }
    var deserialized=[];
    var cIndex=0;
    var attrIndex=0;
    var currObj;
    while(cIndex<odv.byteLength){

      if(attrIndex===classAttrArray.length){
        deserialized.push(currObj);
        attrIndex=0;
      }
      if(attrIndex===0){
        currObj={};
      }
      var thsattrinfo=classAttrArray[attrIndex];

      if(thsattrinfo[1]===this.type.STRING){
        var strLen=odv.getUint32(cIndex,endianness);
        cIndex+=4;

        var str=String.fromCharCode.apply(null, new Uint8Array(odv.buffer   ,cIndex+odv.byteOffset,strLen));
        currObj[thsattrinfo[0]]=str;
        cIndex+=strLen;

      }else if(thsattrinfo[1]===this.type.UINT){
        var val=odv.getUint32(cIndex,endianness);
        currObj[thsattrinfo[0]]=val;

        cIndex+=4;
      }else if(thsattrinfo[1]===this.type.FLOAT){
        var val=odv.getFloat32(cIndex,endianness);
        currObj[thsattrinfo[0]]=val;

        cIndex+=4;
      }else if(thsattrinfo[1]===this.type.INTR){
        var val=odv.getInt32(cIndex,endianness);
        currObj[thsattrinfo[0]]=val;

        cIndex+=4;
      }else if(thsattrinfo[1]===this.type.BYTE){
        var val=odv.getUint8(cIndex,endianness);
        currObj[thsattrinfo[0]]=val;

        cIndex+=1;
      }else if(thsattrinfo[1]===this.type.XYPAIRS){
        var strLen=odv.getUint32(cIndex,endianness);
        cIndex+=4;
        // console.log(odv.buffer ,cIndex+odv.byteOffset,strLen);
        //console.log("import",strLen)
        currObj[thsattrinfo[0]]=new Float32Array(strLen/4);
        for (var i = 0; i < (strLen/4); i++) {
          currObj[thsattrinfo[0]][i]=odv.getFloat32(cIndex,endianness);

          cIndex+=4;
        }
      }
      attrIndex++;
    }
    deserialized.push(currObj);

    return(deserialized);

  }

}
