//unit test of Equation Class
var v=new Vector();
v.resize(51);
var a=new Array();
for(var i=0;i<v.length;i++){
v[i]=Math.random();
a.push(v[i])
}
var x=new Vector(a);
x.tagName="x";

//construct a sequence: -10,-9.5,-9,...,9,9.5,10
var seq=new Array();
k=-10;
for(var i=0;i<=40;i++)
{
  seq.push(k);
  k+=0.5;
}

//construct a sequence:-1,-0.95,-0.9,...,0.95,0.9,1
var seq1=new Array();
k=-1;
for(var i=0;i<=40;i++)
{
seq1.push(k);
k+=0.05;
}
//construct a 0 sequence
var zero=new Array();
for(var i=0;i<=40;i++)
{
zero.push(0);
}

//Test Property: xVectorTest()
function xVectorTest(){
var result=new Array();
/*case1: use the constructor: Equation(equation,xVector[,boolean interpolate])*/
case1=false;
var eq=new Equation("[x]!=0.5",x)
wait();
if(sameVec(x,eq.xVector))
case1=true;
else
result.push("Line 19:xVectorTest() case1->failed\n")

/*case2:use the constructor: Equation(equation,x0,x1,n)*/
case2=false;
var eq=new Equation("0.5",-10,10,41)//xVector for this eq should be x={-10,-9.5,-9,...0...9,9.5,10}
wait();
var xc=new Vector()//construct a x vector for comparsion with eq.xVector
xc.resize(41)
k=-10;
for(var i=0;i<xc.length;i++){xc[i]=k;k=k+0.5;}

if(sameVec(xc,eq.xVector))
case2=true;
else
result.push("Line 33:xVectorTest() case2->failed\n")

//return result
if(case1&&case2)
result.push("xVectorTest() past\n")
else
result.push("xVectorTest() failed\n")
return result;
}

//Test Property: yVectorTest()
function yVectorTest(){
var result=new Array();
/*case1:logical expressions*/
case1=false;
eqx1="[x]>0.5";
eq=new Equation(eqx1,x)
wait();
var yc=new Vector();
yc.resize(x.length);//construct a y vector for comparsion with yVector
yc.zero();
for(var i=0;i<x.length;i++)
{
   if(x[i]>0.5){yc[i]=1}
}
yo=eq.yVector;
if(sameVec(yo,yc))
case1=true;
else
result.push("Line 62:yVectorTest() case1->failed\n")

/*case2:built-in function:ABS()*/
case2=true;
var result=new Array();
var x2=new Vector();
x2.resize(x.length);
x2.tagName="x2";
for(var i=0;i<x.length;i++){
if(x[i]>0.5)
x2[i]=1
else
x2[i]=-1
}

//Apply ABS() to x2
var e2=new Equation("ABS([x2])",x2)
wait();
y2=e2.yVector;
for(var i=0;i<y2.length;i++){
 if(y2[i]!=1)
 {case2=false;
  result.push("Line 84: yVectorTest() case2->failed\n");
  break;}
}

/*case3:built-in function:SQRT()*/
case3=false;
var x3=new Vector();
x3.tagName="x3"
x3.resize(41)
var y3=new Vector();
y3.resize(41)
for(var i=0; i<x3.length;i++)
{
x3[i]=i;
y3[i]=Math.sqrt(x3[i]);
}
var e3=new Equation("SQRT([x3])",x3)
wait();
if(sameVec(y3,e3.yVector))
case3=true;
else
result.push("Line 105:yVectorTest() case3->failed\n")

/*case4:build-in function: SIN()*/
case4=false;
var x4=new Vector(seq);
x4.tagName="x4";
var y4=new Vector(zero);
for(var i=0;i<x4.length;i++)
  y4[i]=Math.sin(x4[i]);

//Apply sin function to x4
e4=new Equation("SIN([x4])",x4)
wait();
if(sameVec(y4,e4.yVector))
case4=true
else
result.push("Line 126:yVectorTest() case4->failed\n")

/*case5:built-in function: COS()*/
case5=false;
var x5=new Vector(seq);x5.tagName="x5"
var y5=new Vector(zero);
for(var i=0;i<seq.length;i++)
  y5[i]=Math.cos(x5[i]);

//Apply cos() to equation
e5=new Equation("COS([x5])",x5);
wait();
if(sameVec(y5,e5.yVector))
case5=true;
else
result.push("Line 152:yVectorTest() case5->failed\n")

/*case6:built-in function: TAN()*/
case6=false;
var x6=new Vector(seq);x6.tagName="x6"
var y6=new Vector(zero);
for(var i=0;i<seq.length;i++)
  y6[i]=Math.tan(x6[i]);

//Apply tan() to equation
e6=new Equation("TAN([x6])",x6)
wait();
if(sameVec(y6,e6.yVector))
case6=true;
else
result.push("Line 165:yVectorTest() case6->failed\n")

/*case7:built-in function ASIN()*/
case7=false;
var x7=new Vector(seq1);x7.tagName="x7";
x7[40]=1;
var y7=new Vector(zero);
for(var i=0;i<seq1.length;i++)
  y7[i]=Math.asin(x7[i]);

//Apply asin() to equation
e7=new Equation("ASIN([x7])",x7)
wait();
if(sameVec(y7,e7.yVector))
case7=true
else
result.push("Line 191:yVectorTest() case7->failed\n")

//return result
if(case1&&case2&&case3&&case4&&case5&&case6&&case7)
result.push("yVectorTest() past\n")
else
result.push("yVectorTest() failed\n")
return result;
}



//alert(xVectorTest()+yVectorTest())
/*helper functions*/
//compare if two vectors have the same entries
function sameVec(v1,v2){
result=true;
   for(var i=0;i<v1.length;i++)
   {
      if(v1[i]!=v2[i])
      {
       result=false;
       break;
      } 
   }   
return result;
}


//wait for a while to let kst update
function wait()
{
var time=0
while(time!=500000)
time++;
}
