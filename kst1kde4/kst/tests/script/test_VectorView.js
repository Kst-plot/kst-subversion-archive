//unit test for the  VectorView Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

v1=staticV(1,10,10);
v1.tagName="x";
v2=v1;
v2.tagName="y";

//test constructor
assert("v=new VectorView(v1,v2)");
assert("v.tagName=\"TestView\"")

//xMin,xMax,yMin,yMax
s1=new Scalar(2);
s2=new Scalar(5);
s3=new Scalar(2);
s4=new Scalar(5);

assert("v.xMin=s1");
assertTrue("v.xMin.value==2");
assert("v.xMax=s2");
assertTrue("v.xMax.value==5");
assert("v.yMin=s3");
assertTrue("v.yMin.value==2")
assert("v.yMax=s4");
assertTrue("v.yMax.value==5");

//useXMin,useXMax,useYMin,useYMax
assertTrue("v.useXMin");
assertTrue("v.useXMax");
assertTrue("v.useYMin");
assertTrue("v.useYMax");

//test flagVector
f=new Vector();
f.resize(10);
f.zero();
f[2]=1;
f[5]=1;
f[8]=1;
assert("v.flagVector=f");

//test x/yVector
assert("v.xVector")
assert("v.yVector")


//return the test result
failed.push(err+" failed test cases found");
if(err>0)
alert(failed)
else
alert("All Tests Past\n")

//if all booleans are true
function alltrue(B)
{
all=true;
 for(var i=0;i<B.length;i++)
 {
	if(!B[i])
	{
	all=false;
	break;
	}
 }
return all;
}

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
//assert code: x works properly without any error
function assert(x,line){
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
