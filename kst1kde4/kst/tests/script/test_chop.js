/*unit test for the chop plugin*/
err=0;
failed=new Array();
failed.push("Failed codes: \n")

//construct an input vector
var in0=new Vector();
in0.resize(1000)
for(var i=0;i<v.length;i++){v[i]=Math.random();}
in0.tagName="in0";

//test using chop plugin through the KstScript
assertNoReturn("p=new Plugin(Kst.pluginManager.modules[\"Chop\"])",32)
assertNoReturn("p.setInput(0,\"in0\")",33);
Kst.waitForUpdate();
assertNoReturn("p.validate()",35);
assertTrue("p.valid",36);
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
