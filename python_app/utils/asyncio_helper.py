import asyncio, threading
class AsyncHelper:
    def __init__(self): self.loop=asyncio.new_event_loop(); self.thread=threading.Thread(target=self._run,daemon=True); self.thread.start()
    def _run(self): asyncio.set_event_loop(self.loop); self.loop.run_forever()
    def run_coroutine(self,coro,cb=None):
        async def w():
            try: r=await coro
            except Exception as e:
                if cb: cb(None,e); return
            if cb: cb(r,None)
        asyncio.run_coroutine_threadsafe(w(),self.loop)
async_helper=AsyncHelper()
