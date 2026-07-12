from kivy.uix.screenmanager import Screen
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.uix.scrollview import ScrollView
from kivy.uix.gridlayout import GridLayout
from kivy.uix.popup import Popup
from utils.asyncio_helper import async_helper
from jnius import autoclass
import json
AudioEngine = autoclass('com.example.voiceclient.AudioEngine')
class GroupsScreen(Screen):
    def __init__(self,backend,**kw): super().__init__(**kw); self.backend=backend; self.groups_list=[]; self.engine=None; self.in_call=False; self.build_ui()
    def build_ui(self):
        l=BoxLayout(orientation='vertical',padding=15,spacing=10)
        l.add_widget(Label(text="المجموعات",font_size='22sp',size_hint_y=None,height=50))
        self.status_lbl=Label(text="جاهز",size_hint_y=None,height=30); l.add_widget(self.status_lbl)
        self.scroll=ScrollView(); self.container=GridLayout(cols=1,spacing=10,size_hint_y=None); self.container.bind(minimum_height=self.container.setter('height')); self.scroll.add_widget(self.container); l.add_widget(self.scroll)
        self.refresh_btn=Button(text="تحديث المجموعات",size_hint_y=None,height=40); self.refresh_btn.bind(on_press=self.load_groups); l.add_widget(self.refresh_btn)
        self.add_widget(l)
    def on_enter(self,*a): self.load_groups(None)
    def start_engine(self):
        if self.engine is None: self.engine=AudioEngine()
        r=self.engine.nativeStart()
        if r==0: self.status_lbl.text="✅ المحرك يعمل"
        else: self.status_lbl.text=f"❌ فشل (كود: {r})"
    def stop_engine(self):
        if self.engine: self.engine.nativeStop(); self.in_call=False; self.status_lbl.text="⏹️ تم الإيقاف"
    def load_groups(self,instance):
        def cb(r,e):
            if e: self.show_error(str(e))
            else: self.groups_list=r; self.update_ui()
        async def l(): return await self.backend.get_groups()
        async_helper.run_coroutine(l(),cb)
    def update_ui(self):
        self.container.clear_widgets()
        for g in self.groups_list:
            box=BoxLayout(orientation='horizontal',size_hint_y=None,height=60,spacing=10)
            box.add_widget(Label(text=g['title'],halign='right',valign='middle'))
            btn=Button(text="الانضمام للمكالمة" if g['has_active_call'] else "لا توجد مكالمة",size_hint_x=None,width=150,background_color=(0.2,0.6,0.2,1) if g['has_active_call'] else (0.5,0.5,0.5,0.3),disabled=not g['has_active_call'])
            btn.bind(on_press=lambda x,gid=g['id']: self.join_call(gid)); box.add_widget(btn)
            self.container.add_widget(box)
    def join_call(self,gid):
        self.start_engine()
        async def get_cfg(): return await self.backend.get_call_config(gid)
        def on_cfg(cfg,e):
            if e: self.status_lbl.text=f"❌ {e}"; return
            r=self.engine.nativeJoinCall(cfg)
            if r==0: self.in_call=True; self.status_lbl.text=f"✅ انضممت للمجموعة {gid}"
            else: self.status_lbl.text=f"❌ فشل الانضمام (كود: {r})"
        async_helper.run_coroutine(get_cfg(),on_cfg)
    def show_error(self,m): Popup(title='خطأ',content=Label(text=m),size_hint=(0.8,0.4)).open()
