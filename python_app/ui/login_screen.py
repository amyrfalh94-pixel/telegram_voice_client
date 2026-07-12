from kivy.uix.screenmanager import Screen
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.uix.textinput import TextInput
from kivy.uix.button import Button
from kivy.uix.popup import Popup
from utils.asyncio_helper import async_helper
from backend import save_credentials, load_credentials
import glob, os

class LoginScreen(Screen):
    def __init__(self,backend,**kw): super().__init__(**kw); self.backend=backend; self.build_ui(); self.try_auto_login()
    def build_ui(self):
        l=BoxLayout(orientation='vertical',padding=30,spacing=15)
        l.add_widget(Label(text="تسجيل الدخول",font_size='24sp',size_hint_y=None,height=50))
        self.api_id_input=TextInput(hint_text="api_id",multiline=False,input_filter='int')
        self.api_hash_input=TextInput(hint_text="api_hash",multiline=False)
        self.phone_input=TextInput(hint_text="رقم الهاتف مع +964...",multiline=False)
        l.add_widget(self.api_id_input); l.add_widget(self.api_hash_input); l.add_widget(self.phone_input)
        self.send_btn=Button(text="إرسال رمز التحقق",size_hint_y=None,height=50); self.send_btn.bind(on_press=self.send_code); l.add_widget(self.send_btn)
        self.code_input=TextInput(hint_text="أدخل الرمز",multiline=False,disabled=True); l.add_widget(self.code_input)
        self.pwd_input=TextInput(hint_text="كلمة مرور 2FA (إن وجدت)",multiline=False,password=True,disabled=True); l.add_widget(self.pwd_input)
        self.confirm_btn=Button(text="تأكيد",size_hint_y=None,height=50,disabled=True); self.confirm_btn.bind(on_press=self.confirm_code); l.add_widget(self.confirm_btn)
        self.status=Label(text='',size_hint_y=None,height=30); l.add_widget(self.status)
        self.add_widget(l)
        sid,shash=load_credentials()
        if sid and shash: self.api_id_input.text=str(sid); self.api_hash_input.text=shash
    def try_auto_login(self):
        from kivy.clock import Clock
        def check(dt):
            ss=glob.glob("sessions/*.session")
            if not ss: return
            ph=os.path.splitext(os.path.basename(ss[0]))[0]
            sid,shash=load_credentials()
            if sid and shash:
                async def init(): await self.backend.init_client(int(sid),shash,ph)
                async_helper.run_coroutine(init(),lambda r,e: Clock.schedule_once(lambda dt: setattr(self.manager,'current','groups') if not e else None))
        Clock.schedule_once(check,1)
    def send_code(self,instance):
        ai=self.api_id_input.text.strip(); ah=self.api_hash_input.text.strip(); ph=self.phone_input.text.strip()
        if not ai or not ah or not ph: self.show_error("املأ جميع الحقول"); return
        try: ai=int(ai)
        except: self.show_error("api_id رقم"); return
        def cb(r,e):
            if e: self.show_error(str(e))
            else: save_credentials(ai,ah); self.code_input.disabled=False; self.confirm_btn.disabled=False; self.pwd_input.disabled=False; self.status.text="تم إرسال الرمز"
        async def s(): await self.backend.init_client(ai,ah,ph); await self.backend.send_code()
        async_helper.run_coroutine(s(),cb)
    def confirm_code(self,instance):
        c=self.code_input.text.strip(); p=self.pwd_input.text.strip() or None
        if not c: self.show_error("أدخل الرمز"); return
        def cb(r,e):
            if e: self.show_error(str(e))
            else: self.manager.current='groups'
        async def co(): await self.backend.sign_in(c, p)
        async_helper.run_coroutine(co(),cb)
    def show_error(self,m): Popup(title='خطأ',content=Label(text=m),size_hint=(0.8,0.4)).open()
