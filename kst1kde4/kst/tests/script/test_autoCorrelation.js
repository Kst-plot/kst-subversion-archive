/*unit test for the Auto-correlation plugin*/
err=0;
failed=new Array();
failed.push("Failed codes: \n")

//construnct an input vector
var v=new Vector();
v.resize(100);
for(var i=0;i<v.length;i++){v[i]=i+1}
v.tagName="v";
eq=new Equation("Sin([v])",v)
in0=eq.yVector;
in0.tagName="in0";

//test using auto-correlation plugin through the KstScript 
assertNoReturn("p=new Plugin(Kst.pluginManager.modules[\"Auto-correlation\"])",16);
assertNoReturn("p.setInput(0,\"in0\")",17);
Kst.waitForUpdate();
assertNoReturn("p.validate()",19);
assertTrue("p.valid",20);
assertNoReturn("xvec=p.inputs[0]",21)
Kst.waitForUpdate();
assertNoReturn("step=p.outputs[0]",23);
Kst.waitForUpdate();
assertNoReturn("auto=p.outputs[1]",25);
Kst.waitForUpdate();


//return the test result
failed.push(err+" failed test cases found");
if(err>0)
alert(failed)
else
alert("All Tests Past\n")

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
            failed.push(x+"\n")
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
                   failed.push("Line "+ line+" "+x+"\n") 
                 }
           }
	catch(e){
		alert("Line: "+line+"Error: " + e.name + "\nLast test was: " + x)
		err++;
                failed.push(x+"\n")
		correct=false;
        } 
        finally{
		return correct;
         }
}
