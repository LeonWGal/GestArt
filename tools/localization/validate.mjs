import fs from 'node:fs';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
const here=path.dirname(fileURLToPath(import.meta.url)),root=path.resolve(here,'../..');
const read=p=>JSON.parse(fs.readFileSync(p,'utf8').replace(/^\uFEFF/,''));
const source=read(path.join(root,'assets/locales-v2/en.json'));
const targets=process.env.TARGET_LOCALES?.split(',')||read(path.join(here,'targets.json'));
const errors=[],warnings=[],locales=[];
const tokens=s=>(s.match(/\{\{[^{}]+\}\}|%\d+|%n|\{[a-zA-Z_][a-zA-Z0-9_]*\}/g)||[]).sort().join('|');
const names=read(path.join(root,'assets/languages.json'));
const expected=['en','ru',...read(path.join(here,'targets.json'))].sort();
if(JSON.stringify(Object.keys(names).sort())!==JSON.stringify(expected))errors.push({problem:'language registry mismatch'});
const actual=fs.readdirSync(path.join(root,'assets/locales-v2')).filter(f=>f.endsWith('.json')).map(f=>f.slice(0,-5)).sort();
if(JSON.stringify(actual)!==JSON.stringify(expected))errors.push({problem:'published catalogs mismatch'});
const catalogs={};
for(const locale of (process.argv.includes('--promote')?targets:['ru',...targets])){
 const file=process.argv.includes('--promote')?path.join(here,'drafts',locale+'.json'):path.join(root,'assets/locales-v2',locale+'.json');
 if(!fs.existsSync(file)){errors.push({locale,problem:'missing catalog'});continue;}
 let data;try{data=read(file);}catch{errors.push({locale,problem:'invalid JSON'});continue;}
 catalogs[locale]=data;
 const missing=Object.keys(source).filter(k=>!(k in data)),extra=Object.keys(data).filter(k=>!(k in source));
 if(missing.length||extra.length)errors.push({locale,problem:'key mismatch',missing,extra});
 for(const [key,english] of Object.entries(source)){
  const value=data[key];if(typeof value!=='string'||!value.trim()){errors.push({locale,key,problem:'empty or non-string translation'});continue;}
  if(tokens(value)!==tokens(english))errors.push({locale,key,problem:'placeholder mismatch'});
  if(value.includes('\ufffd'))errors.push({locale,key,problem:'replacement character'});
  if((value.match(/\n/g)||[]).length!==(english.match(/\n/g)||[]).length)errors.push({locale,key,problem:'newline mismatch'});
  if(value===english&&english.split(/\s+/).length>=4&&!key.startsWith('_plural.'))(process.argv.includes('--strict')||process.argv.includes('--promote')?errors:warnings).push({locale,key,problem:'untranslated sentence'});
  if(english.length>3&&value.length>Math.max(45,english.length*2.8))warnings.push({locale,key,problem:'long translation',characters:value.length});
 }
 locales.push({locale,keys:Object.keys(data).length});
}
const report={sourceKeys:Object.keys(source).length,targetCount:targets.length,locales,errors,warnings};
fs.writeFileSync(path.join(here,'validation-report.json'),JSON.stringify(report,null,2)+'\n');
if(errors.length){console.error(JSON.stringify({ok:false,errors:errors.length,first:errors.slice(0,8)}));process.exitCode=1;}
else{
 if(process.argv.includes('--promote'))for(const [locale,data] of Object.entries(catalogs)){
  const dest=path.join(root,'assets/locales-v2',locale+'.json'),temp=dest+'.tmp';
  fs.writeFileSync(temp,JSON.stringify(Object.fromEntries(Object.keys(source).map(k=>[k,data[k]])),null,2)+'\n');fs.renameSync(temp,dest);
 }
 console.log(JSON.stringify({ok:true,locales:locales.length,keysEach:Object.keys(source).length,warnings:warnings.length,promoted:process.argv.includes('--promote')}));
}
