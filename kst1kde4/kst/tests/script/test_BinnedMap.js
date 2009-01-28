//unit test for the BinnedMap Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

v=staticV(-10,10,1000);
v.tagName="v";
eq=new Equation("[v]+[v]",v);
z=eq.yVector;

assert("b=new BinnedMap()");
assert("b.x=v");
assert("b.y=v");
assert("b.z=z");

assert("b.binnedMap=\"binM\"");
assert("b.hitsMap=\"hitsM\"");

assert("b.xFrom=new Scalar(-1)");
assert("b.xTo=new Scalar(1)");
assert("b.yFrom=new Scalar(-1)");
assert("b.yTo=new Scalar(1)");

assert("b.nX=new Scalar(10)");
assert("b.nY=new Scalar(10)");
assert("b.autobin=new Scalar(1)");

assert("b.validate()");
assertTrue("b.valid");

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
