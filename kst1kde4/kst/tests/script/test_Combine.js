/*unit test for the Combine plugin*/
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
assertNoReturn("p=new Plugin(Kst.pluginManager.modules[\"Combine\"])",25)
assertNoReturn("p.setInput(0,\"in0\")",26);
assertNoReturn("p.setInput(1,\"in1\")",27);
Kst.waitForUpdate();
assertNoReturn("p.validate()",29);
assertTrue("p.valid",30);
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
