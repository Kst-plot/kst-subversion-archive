//unit test for the ELOG class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

//test configuration
assert("e=new ELOG()")
assert("e.hostname=\"142.103.239.68\"");
assert("e.port=8080");
assert("e.logbook=\"demo\"");
assert("e.encodedHTML=true");
assert("e.suppressEmailNotification=true");
assert("e.includeCapture=true");
assert("e.includeConfiguration=true");
assert("e.includeDebugInfo=true");

assertTrue("e.hostname==\"142.103.239.68\"");
assertTrue("e.port==8080");
assertTrue("e.logbook==\"demo\"");
assertTrue("e.encodedHTML");
assertTrue("e.suppressEmailNotification");
assertTrue("e.includeCapture");
assertTrue("e.includeConfiguration");
assertTrue("e.includeDebugInfo");
assertTrue("e.captureWidth==640");
assertTrue("e.captureHeight==480");

assert("e.text=\"ELOG CLass Test\"");
assert("e.addAttribute(\"Author\",\"Kst\")");
assert("e.addAttribute(\"Type\",\"Routine\")");
assert("e.addAttribute(\"Subject\",\"Test Case\")");
assert("e.addAttribute(\"Category\",\"Other\")");
assert("e.submit()");
assert("e.removeAttribute(\"Category\")");
assert("e.submit()");
url="/home/vyiwen/graphics/kst/tests/script/ELOG.txt"
assert("e.addAttachment(url)");
assert("e.submit()");
assert("e.clearAttachments()");
assert("e.submit()");


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
