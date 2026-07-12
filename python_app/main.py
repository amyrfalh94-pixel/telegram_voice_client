from kivy.app import App
from kivy.uix.screenmanager import ScreenManager
from ui.login_screen import LoginScreen
from ui.groups_screen import GroupsScreen
from backend import TelegramController
import os
if not os.path.exists("sessions"): os.makedirs("sessions")
class VoiceClientApp(App):
    def __init__(self,**kw): super().__init__(**kw); self.backend=TelegramController()
    def build(self):
        sm=ScreenManager()
        sm.add_widget(LoginScreen(self.backend,name='login'))
        sm.add_widget(GroupsScreen(self.backend,name='groups'))
        return sm
if __name__=='__main__': VoiceClientApp().run()
