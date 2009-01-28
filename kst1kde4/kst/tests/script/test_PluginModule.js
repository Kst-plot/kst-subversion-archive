//unit test for the PluginModule Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

while((s=prompt("Enter a plugin name or \"q\" to "))!="q")
{
p=new Plugin(Kst.pluginManager.modules[s]);
assertAllProperties(p);
}


function assertAllProperties(p)
{
assert("p.module.usesLoacalData",15);
assert("p.module.isFit",16);
assert("p.module.isFilter",17);
assert("p.module.name",18);
assert("p.module.readableName",19);
assert("p.module.author",20);
assert("p.module.description",21);
assert("p.module.version",22);
assert("p.module.inputs",23);
assert("p.module.outputs",24);
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
