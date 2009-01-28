//unit test for the Arrow Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

var p=new Plot(new Window());

//test Constructors
assert("a=new Arrow(new Window())");
assert("a=new Arrow(p)");
a.to=new Point(100,100);


//fromArrow
assert("a.fromArrow=false");
assertFalse("a.fromArrow");
assert("a.fromArrow=true");
assertTrue("a.fromArrow");

//toArrow
assert("a.toArrow=false")
assertFalse("a.toArrow");
assert("a.toArrow=true");
assertTrue("a.toArrow");

//fromArrowScaling
assertTrue("a.fromArrowScaling==1");
assert("a.fromArrowScaling=5");
assert("a.fromArrowScaling==5");

//toArrowScaling
assertTrue("a.toArrowScaling==1");
assert("a.toArrowScaling=5");
assert("a.toArrowScaling==5");

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
