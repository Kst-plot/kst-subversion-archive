/*unit test for curve class*/
err=0;
failed=new Array();
failed.push("Failed codes: \n")


v0=staticV(-10,10,41); v0.tagName="x"
v1=equationV(v0,"Sin([x])"); v1.tagName="y";
v2=staticV(-1,1,41); v2.tagName="xe";
v3=staticV(-1,1,41); v3.tagName="ye";
v4=staticV(-1,1,41); v4.tagName="x-e";
v5=staticV(-1,1,41); v5.tagName="y-e";
hs=new Histogram(v1);
Kst.waitForUpdate();

/*test constructors*/
assertNoReturn("c=new Curve(v0,v1)",17);
assertNoReturn("c=new Curve(v0,v1,v2,v3)",18);
assertNoReturn("c=new Curve(v0,v1,v2,v3,v4,v5)",19);
assertNoReturn("c=new Curve(\"x\",\"y\",\"xe\",\"ye\",\"x-e\",\"y-e\")",20);
assertNoReturn("h=new Curve(hs)");
Kst.waitForUpdate();

/*test properties*/
/*xVector*/
assertTrue("equalVecs(c.xVector,v0)",26);
assertTrue("equalVecs(h.xVector,hs.xVector)",27);

/*yVector*/
assertTrue("equalVecs(c.yVector,v1)",30);
assertTrue("equalVecs(h.yVector,hs.yVector)",31)

/*x/yErrorVector*/
assertTrue("equalVecs(c.xErrorVector,v2)",34);
assertTrue("equalVecs(c.yErrorVector,v3)",35);

/*x/yMinusErrorVector*/
assertTrue("equalVecs(c.xMinusErrorVector,v4)",38);
assertTrue("equalVecs(c.yMinusErrorVector,v5)",39);

/*color*/
assertNoReturn("c.color=\"#00ff00\"",42);
assertTrue("c.color==\"#00ff00\"",43);
assertNoReturn("c.color=\"purple\"",44);
assertTrue("c.color==\"#a020f0\"",45);

/*samplesPerFrame*/
assertTrue("c.samplesPerFrame==1");

/*ignoreAutoScale*/
assertFalse("c.ignoreAutoScale",51);
assertNoReturn("c.ignoreAutoScale=true",52);
assertTrue("c.ignoreAutoScale",53);
assertNoReturn("c.ignoreAutoScale=false",54);
assertFalse("c.ignoreAutoScale",55);

/*hasPoints/Lines/Bars*/
assertFalse("c.hasPoints",58);
assertTrue("c.hasLines",59);
assertFalse("c.hasBars",60);

assertNoReturn("c.hasPoints=true",62);
assertTrue("c.hasPoints");
assertNoReturn("c.hasLines=false",64);
assertFalse("c.hasLines",65);
assertNoReturn("c.hasBars=true",66);
assertTrue("c.hasBars",67);

/*lineWidth*/
assertTrue("c.lineWidth==1",70);
assertNoReturn("c.lineWidth=5",71);
assertTrue("c.lineWidth==5",72);

/*pointStyle*/
for(var n=0;n<=13;n++)
assertPointStyle(n);

function assertPointStyle(n){
if(!assertNoReturn("c.pointStyle=n",76))
{alert("PointStyle="+n+" failed")};
if(!assertTrue("c.pointStyle==n",76))
{alert("PointStyle="+n+" failed")};
}

/*lineStyle*/
c.hasLines=true;
assertNoReturn("c.lineStyle=0",87);
assertTrue("c.lineStyle==0",88);
assertNoReturn("c.lineStyle=1",89)
assertTrue("c.lineStyle==1",90);
assertNoReturn("c.lineStyle=2",91);
assertTrue("c.lineStyle==2",92);
assertNoReturn("c.lineStyle=3",93);
assertTrue("c.lineStyle==3",94);
assertNoReturn("c.lineStyle=4",95);
assertTrue("c.lineStyle==4",96);

/*barStyle*/
c.hasBars=true;
assertNoReturn("c.barStyle=0",100);
assertTrue("c.barStyle==0",101);
assertNoReturn("c.barStyle=1",102);
assertTrue("c.barStyle==1");

/*pointDensity*/
assertNoReturn("c.pointDensity=0",106);
assertTrue("c.pointDensity==0",107);
assertNoReturn("c.pointDensity=1",108);
assertTrue("c.pointDensity==1",109);
assertNoReturn("c.pointDensity=2",110);
assertTrue("c.pointDensity==2",111)
assertNoReturn("c.pointDensity=3");
assertTrue("c.pointDensity==3",113);


/*Test methods*/
/*point*/
assertTrue("c.point(0).x==v0[0]",118);
assertTrue("c.point(0).y==v1[0]",119);
assertTrue("c.point(11).x==v0[11]",120);
assertTrue("c.point(11).y==v1[11]",121);
assertTrue("c.point(41).x==v0[40]",122);
assertTrue("c.point(41).y==v1[40]",123);

//xErrorPoint
assertTrue("c.xErrorPoint(0)==v2[0]",126);
assertTrue("c.xErrorPoint(11)==v2[11]",127);
assertTrue("c.xErrorPoint(40)==v2[40]",128);


//yErrorPoint
assertTrue("c.yErrorPoint(0)==v3[0]",132);
assertTrue("c.yErrorPoint(1)==v3[1]",133);
assertTrue("c.yErrorPoint(2)==v3[2]",134);

//xMinusErrorPoint
assertTrue("c.xMinusErrorPoint(0)==v4[0]",137);
assertTrue("c.xMinusErrorPoint(11)==v4[11]",138);
assertTrue("c.xMinusErrorPoint(40)==v4[40]",139);

//yMinusErrorPoint
assertTrue("c.yMinusErrorPoint(0)==v5[0]",142);
assertTrue("c.yMinusErrorPoint(11)==v5[11]",143);
assertTrue("c.yMinusErrorPoint(40)==v5[40]",144);

//return the test result
failed.push(err+" failed test cases found");
if(err>0)
alert(failed)
else
alert("All Tests Past\n")
//get a static vector
function staticV(left,right,N){
d=(right-left)/(N-1);
a=new Array();
e=left;

for(var i=0; i<N;i++){
a.push(e);
e=e+d;
}
v=new Vector(a);
return v;
}

//get an equation vector
function equationV(x,equation){
eq=new Equation(equation,x);
return eq.yVector;
}
//compare if two vectors have the same entries
function equalVecs(v1,v2)
{
isequal=true;
if(v1.length!=v2.length)
isequal=false;
 else
 {
   for(var i=0;i<v1.length;i++)
   {
       v1=Math.floor(v1[i]*1E10)
       v2=Math.floor(v2[i]*1E10)
      if(v1!=v2)
       {
         isequal=false;
         break;
        }	
   }	
 }
return isequal;
}
//assert code: x works properly without any error
function assertNoReturn(x,line){
correct=true;
	try{
		eval(x);
         } catch (e) {
		err++;
                failed.push("Line: "+line+" "+x+"\n")
		alert("Line: "+line+" Error: " + e.name + "\nLast test was: " + x);
		
		correct=false;
         }
         finally
            {
              return correct;
            }

}

//assert code: x evalutes to true
function assertTrue(x,line){
correct=true;
	try{
		
		if(!eval(x))
                 { 
                   correct=false;
                   err++;
                   failed.push("Line: "+ line+" "+x+"\n")
                 }   
	}
        catch(e){
            alert("Line: "+line+"Error: " + e.name + "\nLast test was: " + x);
            err++;
            failed.push("Line: "+line+" "+x+"\n")
            correct=false; 
        } 
        finally{
                return correct;
             }
}

/*assert code x evalutes to false*/
function assertFalse(x,line){
correct=true;
	try{
		if(eval(x))
                 {
                   correct=false;
                   err++;
                   failed.push("Line: "+ line+" "+x+"\n") 
                 }
           }
	catch(e){
		alert("Line: "+line+"Error: " + e.name + "\nLast test was: " + x)
		err++;
                failed.push("Line: "+line+x+"\n")
		correct=false;
        } 
        finally{
		return correct;
         }
}
