/*unit test for the Bin plugin*/
err=0;
failed=new Array();
failed.push("Failed codes: \n")

//construct input vectors
v=new Vector();
v.resize(100);
for(var i=0;i<v.length;i++){v[i]=i+1};
v.tagName="v";
eq=new Equation("Sin([v])",v);
in0=eq.yVector;
in0.tagName="in0"

//test using bin plugin throught the KstScript
assert("p=new Plugin(Kst.pluginManager.modules[\"Bin\"])",16);
assert("p.setInput(0,\"in0\")",17);
assert("p.setInput(1,new Scalar(3))",18)
Kst.waitForUpdate();
assert("p.validate()",20);
assertTrue("p.valid",21);
assert("in0=p.inputs[0]",22);
assert("in1=p.inputs[1]",23);
assert("out0=p.outputs[0]",24);
Kst.waitForUpdate();

assert("p.inputs[0]");
assert("p.inputs[1]");
assert("p.outputs[0]");
assert("p.module");

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
