const fs=require('node:fs');
const path=require('node:path');
module.exports={
 entry:path.resolve(__dirname,'../../assets/locales-v2/en.json'),
 entryLocale:'en',
 output:path.resolve(__dirname,'drafts'),
 outputLocales:process.env.TARGET_LOCALES?process.env.TARGET_LOCALES.split(','):require('./targets.json'),
 keyStyle:'flat',
 // Supported token-budget alias; run.mjs sends the configured Gemini model upstream.
 modelName:'gpt-4.1',
 reference:fs.readFileSync(path.join(__dirname,'context.txt'),'utf8'),
 splitToken:2800,
 concurrency:2,
 temperature:0,
 topP:1,
 saveImmediately:true,
 experimental:{jsonMode:true}
};
