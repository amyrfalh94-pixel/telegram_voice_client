from telethon import TelegramClient, errors
from telethon.tl.functions.channels import GetFullChannelRequest
from telethon.tl.functions.messages import GetFullChatRequest
from telethon.tl.types import Channel
from utils.logger import log_event
import time, json, os, base64

CONFIG_FILE = "config.txt"

def save_credentials(api_id, api_hash):
    with open(CONFIG_FILE, "w") as f:
        f.write(base64.b64encode(f"{api_id}:{api_hash}".encode()).decode())

def load_credentials():
    if not os.path.exists(CONFIG_FILE): return None, None
    try:
        with open(CONFIG_FILE) as f:
            dec = base64.b64decode(f.read().encode()).decode()
            aid, ahash = dec.split(":", 1)
            return int(aid), ahash
    except: return None, None

class TelegramController:
    def __init__(self): self.client=None; self.api_id=None; self.api_hash=None; self.phone=None
    async def init_client(self,api_id,api_hash,phone): 
        self.api_id=api_id; self.api_hash=api_hash; self.phone=phone
        self.client=TelegramClient(f'sessions/{phone}',api_id,api_hash)
        await self.client.connect()
    async def send_code(self): 
        return await self.client.send_code_request(self.phone)
    async def sign_in(self,code, password=None):
        try:
            await self.client.sign_in(self.phone, code)
        except errors.SessionPasswordNeededError:
            if password:
                await self.client.sign_in(password=password)
            else:
                raise
    async def is_authorized(self): 
        return await self.client.is_user_authorized()
    async def get_groups(self):
        if not await self.is_authorized():
            return []
        start=time.time(); dialogs=await self.client.get_dialogs(); groups=[]
        for d in dialogs:
            if d.is_group or d.is_channel:
                try:
                    if isinstance(d.entity,Channel): full=await self.client(GetFullChannelRequest(channel=d.entity))
                    else: full=await self.client(GetFullChatRequest(chat_id=d.entity.id))
                    active=full.full_chat.call is not None
                except: active=False
                groups.append({'id':d.entity.id,'title':d.entity.title,'has_active_call':active})
        log_event(f"تم جلب {len(groups)} مجموعة في {(time.time()-start)*1000:.0f}ms")
        return groups
    async def get_call_config(self, group_id): return json.dumps({"group_id":group_id,"protocol":"tgvoip","sample_rate":48000,"channels":1})
