//unit test for the Box Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

p=new Plot(new Window());

//test constructors
assert("b=new Box(\"W1\")");
assert("b=new Box(p)");
b.resize(100,100);
//xRound, yRound
assertTrue("b.xRound==0");
assertTrue("b.yRound==0");

assert("b.xRound=99");
assert("b.yRound=99");
assertTrue("b.xRound==99&&b.yRound==99");

assert("b.xRound=10");
assert("b.yRound=10");
assertTrue("b.xRound==10&&b.yRound==10");

assert("b.xRound=0");
assert("b.yRound=0");
assertTrue("b.xRound==0&&b.yRound==0");


//borderColor
assertTrue("b.borderColor==\"#000000\"");
assert("b.borderColor=\"#ff1064\"");
assertTrue("b.borderColor==\"#ff1064\"");

//borderWidth
assertTrue("b.borderWidth==0");
assert("b.borderWidth=3");
assertTrue("b.borderWidth==3");


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
