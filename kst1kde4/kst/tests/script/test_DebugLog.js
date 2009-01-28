//unit test for DebugLog

err=0;
failed=new Array();
failed.push("Failed codes: \n")


//test properties
//keep prompting users for getting .kst file to test until the user enters "q"
while(fn!="q")
{
reply=prompt("Enter a kst file name to test or q to quit")
fn = reply;
if(fn=="q")
break;

Kst.document.load(fn);

assert("Debug.log.length");
assert("Debug.log.text");
assert("Debug.log.lengthNotices");
assert("Debug.log.textNotices");
assert("Debug.log.lengthWarnings");
assert("Debug.log.textWarnings");
assert("Debug.log.lengthErrors");
assert("Debug.log.textErrors");
assert("Debug.log.lengthDebugs");
assert("Debug.log.textDebugs");

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
