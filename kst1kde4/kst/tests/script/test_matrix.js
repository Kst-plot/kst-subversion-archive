/*unit test for the Matrix Class*/


err=0;
function assert(x) {
xrc=false;
	try {
		var xrc = eval(x);
                if(!xrc){
                err++;	
                 } 
		
	} catch (e) {
		alert("Error: " + e.name + "\nLast test was: " + x);
		err++;
	}
        finally{
               return xrc;
                }

}

function assertNoReturn(x){
correct=true;
	try{
		eval(x);
         } catch (e) {
		alert("Error: " + e.name + "\nLast test was: " + x);
		err++;
		correct=false;
         }
         finally
            {
              return correct;
            }

}

//test constructor Matrix()
x="mx=new Matrix()"
if(!assertNoReturn(x)){alert("Line 41: Failed: "+x)}

//test Property: editable
x="mx.editable"
if(!assert(x)){alert("Line 44: Failed: "+x)}

//test method: value(column,row)
x="mx.value(0,0)==0"
if(!assert(x)){"Line 49: Failed: "+x}

//test method: setValue(column,row,value)
value=Math.random();
x="mx.setValue(0,0,value)"
if(!assert(x)){alert("Line 54: Failed: "+x)}

//test Method resize(rows,cols)
x="mx.resize(10,5)"
if(!assertNoReturn(x)){alert("Line 58: Failed: "+x)}

//test Property: columns
x="mx.columns==5";
if(!assert(x)){alert("Line 62: Failed: "+x)}

//test Property rows
x="mx.rows==10"
if(!assert(x)){alert("Line 66: Failed: "+x)}

//test Property: min
entries=new Array();
for(var i=0; i<5; i++)
{
   for(var j=0;j<10;j++)
    {mx.setValue(i,j,Math.random());
     entries.push(mx.value(i,j))} 
}
entries.sort();
x="mx.min==entries[0]"
if(!assert(x)){alert("Line 78: Failed: "+x)}

//test Property: max
entries.reverse();
x="mx.max==entries[0]"
if(!assert(x)){alert("Line 83: Failed: "+x)}

//test Property:mean
mx_entries=new Vector(entries)
x="Math.ceil(mx.mean*1E15)==Math.ceil(mx_entries.mean*1E15)"//check to the postion 10^-15
if(!assert(x)){alert("Line 88: Failed: "+x)}

//test Method update()
x="mx.update()"
if(!assertNoReturn(x)){alert("Line 92: Failed: "+x)}


//return test result
if(err>0){
alert(" "+err+"failed TestCases");
}
else
alert("All Test Past");
