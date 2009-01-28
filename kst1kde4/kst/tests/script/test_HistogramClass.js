/*unit test for the Histogram class*/
err=0;
failed=new Array();
failed.push("Failed codes: \n")

v=new Vector();
v.resize(1000);
for(var i=0;i<1000;i++){v[i]=Math.random();}

//test constructors:
assert("hs1=new Histogram(v)");
hs1.tagName="hs1";
assert("hs2=new Histogram(v,-0.5,0.5,10)");
hs2.tagName="hs2";

//realTimeAutoBin
assert("hs1.realTimeAutoBin=true");
assertTrue("hs1.realTimeAutoBin");
assert("hs1.realTimeAutoBin=false");
assertFalse("hs1.realTimeAutoBin");

//normalization
assert("hs1.normalization=0");
assert("hs1.normalization=1");
assert("hs1.normalization=2");
assert("hs1.normalization=3");

//bins
assertTrue("hs1.bins==60");
assert("hs1.bins=1");
assertTrue("hs1.bins==2");
assert("hs1.bins=10");
assertTrue("hs1.bins==10");

//xMin
assertTrue("hs1.xMin=v.min");

//xMax
assertTrue("hs1.xMax=v.max");

//xVector
assert("hs1.xVector");

//yVector
assert("hs1.yVector");

//setVector();
v2=staticV(1,5,10001);
assert("hs1.setVector(v2)");

//setRange();
assert("hs1.setRange(2,3)");

//return the test result
failed.push(err+" failed test cases found");
if(err>0)
alert(failed)
else
alert("All Tests Past\n")
//help functions
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
function assert(x,line){
correct=true;
	try{
		eval(x);
         } catch (e) {
		err++;
                failed.push("Number: "+line+" "+x+"\n")
		alert("Number: "+line+" Error: " + e.name + "\nLast test was: " + x);
		
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
                   failed.push("Number: "+ line+" "+x+"\n")
                 }   
	}
        catch(e){
            alert("Number: "+line+"Error: " + e.name + "\nLast test was: " + x);
            err++;
            failed.push("Number: "+line+" "+x+"\n")
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
                   failed.push("Number: "+ line+" "+x+"\n") 
                 }
           }
	catch(e){
		alert("Number: "+line+"Error: " + e.name + "\nLast test was: " + x)
		err++;
                failed.push("Number: "+line+x+"\n")
		correct=false;
        } 
        finally{
		return correct;
         }
}
