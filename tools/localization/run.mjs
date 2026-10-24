import fs from 'node:fs';
import path from 'node:path';
import http from 'node:http';
import crypto from 'node:crypto';
import {fileURLToPath} from 'node:url';
import {spawn} from 'node:child_process';
import dotenv from 'dotenv';
const here=path.dirname(fileURLToPath(import.meta.url));
dotenv.config({path:path.join(here,'.env'),quiet:true});
const key=process.env.OPENAI_API_KEY;
if(!key){console.error('Set OPENAI_API_KEY in tools/localization/.env first.');process.exit(2);}
const root=path.resolve(here,'../..');
fs.mkdirSync(path.join(here,'drafts'),{recursive:true});
const selectedTargets=process.env.TARGET_LOCALES?.split(',')||JSON.parse(fs.readFileSync(path.join(here,'targets.json'),'utf8'));
for(const locale of selectedTargets){
 const draft=path.join(here,'drafts',locale+'.json'),published=path.join(root,'assets/locales-v2',locale+'.json');
 if(!fs.existsSync(draft)&&fs.existsSync(published))fs.copyFileSync(published,draft);
}

const model=process.env.GEMINI_MODEL||'gemini-3.5-flash-lite';
const upstream='https://generativelanguage.googleapis.com/v1beta/openai/chat/completions';
const nonce=crypto.randomBytes(24).toString('hex');
let active=0,completed=0;const queue=[];const stop=new AbortController();
async function acquire(){if(active>=2)await new Promise(resolve=>queue.push(resolve));active++;}
function release(){active--;queue.shift()?.();}
const server=http.createServer(async(req,res)=>{
 if(req.method!=='POST'||req.url!=='/v1/chat/completions'||req.headers.authorization!==`Bearer ${nonce}`){res.writeHead(404);res.end();return;}
 let entered=false;
 try{
  let raw='';for await(const chunk of req){raw+=chunk;if(raw.length>2_000_000)throw new Error('Request too large');}
  const body=JSON.parse(raw);body.model=model;body.reasoning_effort="low";
  await acquire();entered=true;
  const response=await fetch(upstream,{method:'POST',headers:{'Content-Type':'application/json',Authorization:`Bearer ${key}`},body:JSON.stringify(body),signal:AbortSignal.any([stop.signal,AbortSignal.timeout(65000)])});
  let text=(await response.text()).replaceAll(key,'[redacted]');
  if(response.ok){
   try{const envelope=JSON.parse(text);const content=envelope.choices?.[0]?.message?.content;const translated=JSON.parse(content);if(!translated||Array.isArray(translated)||typeof translated!=='object')throw new Error('not a JSON object');}
   catch{completed++;console.log(`Gemini: request ${completed}, invalid JSON; retry requested`);res.writeHead(502,{'Content-Type':'application/json'});res.end(JSON.stringify({error:{message:'Translation response was not valid JSON; retry this chunk.'}}));return;}
  }
  res.writeHead(response.status,{'Content-Type':'application/json'});res.end(text);
  completed++;console.log(`Gemini: request ${completed}, HTTP ${response.status}`);if(!response.ok)fs.writeFileSync(path.join(here,'provider-error.json'),text);
 }catch(error){console.log('Gemini: request timed out or failed');res.writeHead(502,{'Content-Type':'application/json'});res.end(JSON.stringify({error:{message:String(error.message).replaceAll(key,'[redacted]')}}));}
 finally{if(entered)release();}
});
await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));
const env={...process.env,OPENAI_API_KEY:nonce,OPENAI_PROXY_URL:`http://127.0.0.1:${server.address().port}/v1`};
const cli=path.join(here,'node_modules/@lobehub/i18n-cli/dist/cli.js');
const child=spawn(process.execPath,[cli,'locale','--config',path.join(here,'.i18nrc.cjs')],{cwd:here,env,stdio:['ignore','pipe','pipe'],windowsHide:true});
let log='',finished=false;const capture=chunk=>{
 log+=chunk.toString().replaceAll(key,'[redacted]').replaceAll(nonce,'[local-token]');
 fs.writeFileSync(path.join(here,'translation.log'),log);
 // Ink in the pinned CLI can retain a render timer after its completion message.
 if(!finished&&log.includes('All i18n tasks have been completed')){finished=true;setTimeout(()=>child.kill(),300).unref();}
};child.stdout.on('data',capture);child.stderr.on('data',capture);
const exit=await new Promise(resolve=>{child.on('close',resolve);child.on('error',e=>{capture(Buffer.from(e.message));resolve(1);});});
const code=finished?0:(exit??1);
stop.abort();server.close();
fs.writeFileSync(path.join(here,'translation.log'),log);
const info={tool:'@lobehub/i18n-cli@1.27.0',provider:'Gemini',model,requests:completed,exitCode:code,targets:process.env.TARGET_LOCALES?.split(',')||JSON.parse(fs.readFileSync(path.join(here,'targets.json'),'utf8'))};
fs.writeFileSync(path.join(here,'last-run.json'),JSON.stringify(info,null,2)+'\n');
console.log(JSON.stringify(info));
if(code!==0)console.error(log.replace(/\x1b\[[0-9;]*m/g,'').slice(-900));
process.exitCode=code||0;

