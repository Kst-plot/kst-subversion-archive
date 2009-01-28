//test Spectrogram
err=0;
failed=new Array();
failed.push("Failed codes: \n")

var v0=staticV(-10,10,1000);
v0.tagName="t";
var v1=equationV(v0,"Sin([t])");

assert("spg=new Spectrogram(v1,10)");
assert("spg.tagName=\"Test_Spectrogram\"");
assert("spg.length=10");
assert("spg.average=false");
assert("spg.apodize=false");
assert("spg.removeMean=false");
assert("spg.windowSize=500");

assertTrue("spg.length==10");
assertTrue("spg.frequency==10");
assertFalse("spg.removeMean");
assertFalse("spg.average");
assertFalse("spg.apodize");
assertTrue("spg.vUnits==\"V\"");
assertTrue("spg.rUnits==\"Hz\"");
assertTrue("spg.windowSize==500");

assert("mx=spg.matrix");
assert("mx.tagName=\"Test_Matrix\"");

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
