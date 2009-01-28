/*unit test for the Convolution plugin*/
err=0;
failed=new Array();
failed.push("Failed codes: \n")

//construct test vectors
var v1=new Vector();
v1.resize(50)
for(var i=0;i<v1.length;i++){v1[i]=Math.random();}
v1.tagName="in0";

var v2=new  Vector();
v2.resize(50)
for(var i=0;i<v2.length;i++){v2[i]=Math.random();}
v2.tagName="in1"

//test using chop plugin through the KstScript
assert("p=new Plugin(Kst.pluginManager.modules[\"Convolution\"])",18)
assert("p.setInput(0,\"in0\")",19);
assert("p.setInput(1,\"in1\")",20);
Kst.waitForUpdate();
assert("p.validate()",22);
assertTrue("p.valid",23);
Kst.waitForUpdate();

assert("p.inputs[0]")
assert("p.inputs[1]");
assert("p.outputs[0]")

//return the test result
failed.push(err+" failed test cases found");
if(err>0)
alert(failed)
else
alert("All Tests Past\n")

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
