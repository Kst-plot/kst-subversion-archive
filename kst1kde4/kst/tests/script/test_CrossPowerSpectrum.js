//unit test for the CrossPowerSpectrum Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

v=staticV(-10,10,101)
v.tagName="v"
eq=new Equation("Sin([v])",v);

assert("c=new CrossPowerSpectrum()");
assert("c.v1=eq.xVector");
assert("c.v2=eq.yVector");

assert("c.length=new Scalar(10)");
assertTrue("c.length.value==10");
assert("c.sample=new Scalar(10)");
assertTrue("c.sample.value==10");
assert("c.real=\"rl\"");
assert("c.frequency=\"fr\"");
assert("c.imaginary=\"im\"");
assert("c.validate()");
assertTrue("c.valid");

r=new Vector();
i=new Vector();
f=new Vector();

assert("r=c.real");
assert("i=c.imaginary");
assert("f=c.frequency");

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
