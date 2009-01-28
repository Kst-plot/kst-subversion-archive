//unit test for the Document Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")


var d=Kst.document
B=new Array()

while((s=prompt("Enter a kst file name or \"q\" to exit"))!="q")
{
//test load(filename)
B.push(assert("d.load(s)"))

//text
B.push(assert("d.text"));

//name
B.push(assert("d.name"));

//modified
B.push(assertFalse("d.modified"));
c=new Curve(Kst.vectors[0],Kst.vectors[0]);
p=new Plot(new Window());
p.curves.append(c);
B.push(assertTrue("d.modified"));

//save();
B.push(assert("d.save()"));
url=prompt("Enter a file path and name to save the file. e.g ~/test.kst")
B.push(assert("d.save(url)"));
B.push(assert("d.load(url)"));

if(!alltrue(B))
{
failed.push("Failed test case is: "+s+"\n");
break;
}

}



//return the test result
failed.push(err+" failed test cases found");
if(err>0)
alert(failed)
else
alert("All Tests Past\n")


//if all booleans are true
function alltrue(B)
{
all=true;
 for(var i=0;i<B.length;i++)
 {
	if(!B[i])
	{
	all=false;
	break;
	}
 }
return all;
}



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
