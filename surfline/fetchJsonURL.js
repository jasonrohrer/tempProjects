const puppeteer = require('puppeteer');
const fs = require('fs');
const fsp = require('fs').promises;

if( process.argv.length < 3 ) {
	console.log( "Missing URL arg" );
	process.exit();
}

const url = process.argv[2];


(async () => {

	// use system-installed browser (one installed by puppeteer has
	// dependency issues)
	const browser = await puppeteer.launch(
		{ executablePath:'/usr/bin/chromium-browser' });

	const page = await browser.newPage();

	// Set screen size
	await page.setViewport({width: 1280, height: 720});

	// bypass allow-cookie confirmation dialog box
	await page.setCookie( { 'name': 'necessaryCookiesAllowed',
							'value': 'yes',
							'domain': 'login.reconyx.com' } );
	await page.setCookie( { 'name': 'necessaryCookiesAllowed',
							'value': 'yes',
							'domain': 'connect.reconyx.com' } );
	
	
	var response = await page.goto( url ,
									{ waitUntil: 'networkidle2' } );
	
	const text = await response.text();
	
	process.stdout.write( text );

	await browser.close();
	
})().catch( e => { console.error(e); 
				   process.exit(1); } );