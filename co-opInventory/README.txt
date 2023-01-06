This is a simple app that lets you enter quantity-price-quantity-price...
instead of entering those pesky Excell formulas (=5*55.23) over and over.


Run inventory.exe by double-clicking it.

It will ask you for the name of a file to save data in.  An example file name
might be:

2005_inventory.txt

The file name cannot contain any spaces.



 --
| Entering quantities and prices
 --

After creating the file, the app will start asking you for quantities and
prices.  Type a quantity, then hit Enter, then type the corresponding price
and hit Enter again.



 --
| Starting a new page
 --

When you come to the end of a page, enter a 
p
instead of a quantity to start a new page in the app.



 --
| Quitting and exporting to Excell
 --

When you are done (or to save your progress), enter a 
q
instead of a quantity to quit.  Upon quitting, the app exports a CSV file
(comma separated values) that can be imported into Excell.



 --
| Resuming
 --

To resume where you left off after quitting, enter the name of your previous
data file.  The app will read your file and then start adding new quantities
where you left off.



 --
| Fixing mistakes.
 --

If you make a mistake, you can always edit the data file by hand.  It is
a plain text file that can be opened with Notepad or another text editor.
You can fix bad entries in this way.  However, to generate a new CSV file
after you fix mistakes, you will need to launch the app again, re-open
the modified data file, and then quit (the app always saves a fresh CSV file
before exiting).
