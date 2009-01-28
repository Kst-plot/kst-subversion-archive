//unit test for DebugLogEntry
err=0;
failed=new Array();
failed.push("Failed codes: \n")


var fn="kstfilename";

//test properties
while(fn!="q")
{
reply=prompt("Enter a kst file name or q to quit:")
fn = reply;
if(fn == "q")
break;
else
Kst.document.load(fn);
for(var i=0;i<Debug.log.length;i++)
{
assert("Debug.log[i].text");
assert("Debug.log[i].date");
assert("Debug.log[i].level");
}

}

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
