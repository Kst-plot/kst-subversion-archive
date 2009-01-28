//unit test for the Label Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

p=new Plot(new Window());
//test Consturctors
assert("l=new Label(\"W1\")");
assert("l=new Label(p)");

//text
text="Label Test:\n\\delta,\\gamma,\\inf,\\suma_{1}y^{2}\n1234567890";
assert("l.text=text");

//font
assert("l.font=\"Luxi Serif\"");
assertTrue("l.font==\"Luxi Serif\"");

//fontSize
assert("l.fontSize=12");
assertTrue("l.fontSize==12");

//rotation
assert("l.rotation=90");
assertTrue("l.rotation==90");
assert("l.rotation=-45");
assertTrue("l.rotation==-45");


//interpreted
assertTrue("l.interpreted");

//scalarReplacement
assertTrue("l.scalarReplacement");

//adjustSizeForText()
assert("l.adjustSizeForText()");
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
