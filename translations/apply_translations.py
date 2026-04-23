#!/usr/bin/env python3
"""Fill hand-written translations into the lupdate-generated .ts files.

Each language has a dict mapping the English source string to its
translation. Strings missing from the dict are left as
`type="unfinished"` (Qt falls back to the source). Entries whose
translation is the empty string (product name, numeric placeholders)
are emitted as empty translations so Qt falls back to the source.
"""

from __future__ import annotations

import re
from pathlib import Path
from xml.sax.saxutils import escape as xml_escape

ROOT = Path(__file__).parent

# The 37 strings extracted by lupdate.
# Keys are the *unescaped* source strings (exactly as written in C++ / .ui).
# Any key missing from a language dict is left unfinished (falls back to source).

NO_TRANSLATE = {"QMineSweeper", "10", "000.0"}

EN: dict[str, str] = {
    # English is the source language; we ship no translations so Qt uses
    # the source strings directly.
}

TR: dict[str, str] = {
    "&About": "&Hakkında",
    "&Beginner  (9×9, 10 mines)": "&Başlangıç  (9×9, 10 mayın)",
    "&Difficulty": "&Zorluk",
    "&Expert  (30×16, 99 mines)": "&Uzman  (30×16, 99 mayın)",
    "&Game": "&Oyun",
    "&Help": "&Yardım",
    "&Intermediate  (16×16, 40 mines)": "&Orta  (16×16, 40 mayın)",
    "&Language": "&Dil",
    "&New": "&Yeni",
    "&Quit": "&Çıkış",
    "&Settings": "&Ayarlar",
    "<h3>QMineSweeper %1</h3><p>A Qt6-based Minesweeper game.</p><p>Left-click to reveal, right-click to flag, middle-click on a satisfied number to chord.</p><p>© Mavrikant</p>":
        "<h3>QMineSweeper %1</h3><p>Qt6 tabanlı bir Mayın Tarlası oyunu.</p><p>Açmak için sol tık, bayraklamak için sağ tık, sayıyı tamamlayan bayrakların komşularını açmak için orta tık.</p><p>© Mavrikant</p>",
    "About QMineSweeper": "QMineSweeper Hakkında",
    "Auto (system)": "Otomatik (sistem)",
    "Boom": "Bom!",
    "Close": "Kapat",
    "Help improve QMineSweeper": "QMineSweeper’ı iyileştirmeye yardımcı olun",
    "Language changed": "Dil değiştirildi",
    "Language changes take effect after restart.": "Dil değişiklikleri yeniden başlatıldıktan sonra geçerli olur.",
    "Later": "Sonra",
    "New Game": "Yeni Oyun",
    "No thanks": "Hayır, teşekkürler",
    "QMineSweeper — Boom": "QMineSweeper — Bom!",
    "QMineSweeper — Playing": "QMineSweeper — Oynanıyor",
    "QMineSweeper — You won!": "QMineSweeper — Kazandınız!",
    "Restart QMineSweeper now?": "QMineSweeper şimdi yeniden başlatılsın mı?",
    "Restart now": "Şimdi yeniden başlat",
    "Send anonymous &crash reports and usage data": "Anonim &çökme raporları ve kullanım verisi gönder",
    "We collect: app crashes, game results (win/loss, duration, difficulty), OS name, CPU architecture, Qt version, and an anonymous install ID.<br/><br/>We do <b>not</b> collect: your name, email, IP address, file paths, or any in-game actions.<br/><br/>You can change this later in <b>Settings</b>.":
        "Topladıklarımız: uygulama çökmeleri, oyun sonuçları (galibiyet/mağlubiyet, süre, zorluk), işletim sistemi adı, CPU mimarisi, Qt sürümü ve anonim bir yükleme kimliği.<br/><br/><b>Toplamadıklarımız:</b> adınız, e-posta adresiniz, IP adresiniz, dosya yolları veya oyun içi hareketler.<br/><br/>Bu tercihi daha sonra <b>Ayarlar</b>’dan değiştirebilirsiniz.",
    "Would you like to send anonymous crash reports and usage data?": "Anonim çökme raporları ve kullanım verisi göndermek ister misiniz?",
    "Yes, send": "Evet, gönder",
    "You cleared the field in %1 seconds.": "Alanı %1 saniyede temizlediniz.",
    "You stepped on a mine.": "Bir mayına bastınız.",
    "You won!": "Kazandınız!",
    "&Statistics…": "&İstatistikler…",
    "Statistics": "İstatistikler",
    "Difficulty": "Zorluk",
    "Played": "Oynanan",
    "Won": "Kazanılan",
    "Best time": "En iyi süre",
    "Beginner": "Başlangıç",
    "Intermediate": "Orta",
    "Expert": "Uzman",
    "Reset all": "Tümünü sıfırla",
    "Reset statistics?": "İstatistikler sıfırlansın mı?",
    "Permanently erase all played / won / best-time records?": "Tüm oynanmış / kazanılmış / en iyi süre kayıtları kalıcı olarak silinsin mi?",
    "🏆 New record!": "🏆 Yeni rekor!",
    "<p><small>Built with Qt %1 on %2</small></p>": "<p><small>Qt %1 ile %2 tarihinde derlendi</small></p>",
    "Enable &question marks": "&Soru işaretlerini etkinleştir",
}

ES: dict[str, str] = {
    "&About": "&Acerca de",
    "&Beginner  (9×9, 10 mines)": "&Principiante  (9×9, 10 minas)",
    "&Difficulty": "&Dificultad",
    "&Expert  (30×16, 99 mines)": "&Experto  (30×16, 99 minas)",
    "&Game": "&Juego",
    "&Help": "A&yuda",
    "&Intermediate  (16×16, 40 mines)": "&Intermedio  (16×16, 40 minas)",
    "&Language": "&Idioma",
    "&New": "&Nuevo",
    "&Quit": "&Salir",
    "&Settings": "A&justes",
    "<h3>QMineSweeper %1</h3><p>A Qt6-based Minesweeper game.</p><p>Left-click to reveal, right-click to flag, middle-click on a satisfied number to chord.</p><p>© Mavrikant</p>":
        "<h3>QMineSweeper %1</h3><p>Un juego de Buscaminas basado en Qt6.</p><p>Clic izquierdo para revelar, clic derecho para marcar con bandera, clic central sobre un número satisfecho para despejar alrededor.</p><p>© Mavrikant</p>",
    "About QMineSweeper": "Acerca de QMineSweeper",
    "Auto (system)": "Automático (sistema)",
    "Boom": "¡Bum!",
    "Close": "Cerrar",
    "Help improve QMineSweeper": "Ayuda a mejorar QMineSweeper",
    "Language changed": "Idioma cambiado",
    "Language changes take effect after restart.": "Los cambios de idioma se aplican al reiniciar.",
    "Later": "Más tarde",
    "New Game": "Nueva partida",
    "No thanks": "No, gracias",
    "QMineSweeper — Boom": "QMineSweeper — ¡Bum!",
    "QMineSweeper — Playing": "QMineSweeper — Jugando",
    "QMineSweeper — You won!": "QMineSweeper — ¡Has ganado!",
    "Restart QMineSweeper now?": "¿Reiniciar QMineSweeper ahora?",
    "Restart now": "Reiniciar ahora",
    "Send anonymous &crash reports and usage data": "Enviar informes de &fallos y datos de uso anónimos",
    "We collect: app crashes, game results (win/loss, duration, difficulty), OS name, CPU architecture, Qt version, and an anonymous install ID.<br/><br/>We do <b>not</b> collect: your name, email, IP address, file paths, or any in-game actions.<br/><br/>You can change this later in <b>Settings</b>.":
        "Recopilamos: fallos de la aplicación, resultados de partidas (victoria/derrota, duración, dificultad), nombre del sistema operativo, arquitectura de CPU, versión de Qt y un identificador de instalación anónimo.<br/><br/>No recopilamos: su nombre, correo electrónico, dirección IP, rutas de archivos ni acciones dentro del juego.<br/><br/>Puede cambiar esta opción más tarde en <b>Ajustes</b>.",
    "Would you like to send anonymous crash reports and usage data?": "¿Desea enviar informes de fallos y datos de uso anónimos?",
    "Yes, send": "Sí, enviar",
    "You cleared the field in %1 seconds.": "Ha despejado el campo en %1 segundos.",
    "You stepped on a mine.": "Ha pisado una mina.",
    "You won!": "¡Ha ganado!",
    "&Statistics…": "&Estadísticas…",
    "Statistics": "Estadísticas",
    "Difficulty": "Dificultad",
    "Played": "Jugadas",
    "Won": "Ganadas",
    "Best time": "Mejor tiempo",
    "Beginner": "Principiante",
    "Intermediate": "Intermedio",
    "Expert": "Experto",
    "Reset all": "Borrar todo",
    "Reset statistics?": "¿Borrar estadísticas?",
    "Permanently erase all played / won / best-time records?": "¿Borrar permanentemente todos los registros de jugadas / victorias / mejores tiempos?",
    "🏆 New record!": "🏆 ¡Nuevo récord!",
    "<p><small>Built with Qt %1 on %2</small></p>": "<p><small>Compilado con Qt %1 el %2</small></p>",
    "Enable &question marks": "Habilitar signos de &interrogación",
}

FR: dict[str, str] = {
    "&About": "&À propos",
    "&Beginner  (9×9, 10 mines)": "&Débutant  (9×9, 10 mines)",
    "&Difficulty": "&Difficulté",
    "&Expert  (30×16, 99 mines)": "&Expert  (30×16, 99 mines)",
    "&Game": "&Jeu",
    "&Help": "&Aide",
    "&Intermediate  (16×16, 40 mines)": "&Intermédiaire  (16×16, 40 mines)",
    "&Language": "&Langue",
    "&New": "&Nouveau",
    "&Quit": "&Quitter",
    "&Settings": "&Paramètres",
    "<h3>QMineSweeper %1</h3><p>A Qt6-based Minesweeper game.</p><p>Left-click to reveal, right-click to flag, middle-click on a satisfied number to chord.</p><p>© Mavrikant</p>":
        "<h3>QMineSweeper %1</h3><p>Un jeu de Démineur basé sur Qt6.</p><p>Clic gauche pour révéler, clic droit pour poser un drapeau, clic milieu sur un nombre satisfait pour enchaîner les voisines.</p><p>© Mavrikant</p>",
    "About QMineSweeper": "À propos de QMineSweeper",
    "Auto (system)": "Auto (système)",
    "Boom": "Boum !",
    "Close": "Fermer",
    "Help improve QMineSweeper": "Aidez à améliorer QMineSweeper",
    "Language changed": "Langue modifiée",
    "Language changes take effect after restart.": "Les changements de langue prennent effet après redémarrage.",
    "Later": "Plus tard",
    "New Game": "Nouvelle partie",
    "No thanks": "Non, merci",
    "QMineSweeper — Boom": "QMineSweeper — Boum !",
    "QMineSweeper — Playing": "QMineSweeper — En partie",
    "QMineSweeper — You won!": "QMineSweeper — Vous avez gagné !",
    "Restart QMineSweeper now?": "Redémarrer QMineSweeper maintenant ?",
    "Restart now": "Redémarrer maintenant",
    "Send anonymous &crash reports and usage data": "Envoyer des rapports de &plantage et données d’usage anonymes",
    "We collect: app crashes, game results (win/loss, duration, difficulty), OS name, CPU architecture, Qt version, and an anonymous install ID.<br/><br/>We do <b>not</b> collect: your name, email, IP address, file paths, or any in-game actions.<br/><br/>You can change this later in <b>Settings</b>.":
        "Nous collectons : les plantages de l’application, les résultats de partie (victoire/défaite, durée, difficulté), le nom du système d’exploitation, l’architecture du processeur, la version de Qt et un identifiant d’installation anonyme.<br/><br/>Nous ne collectons <b>pas</b> : votre nom, votre e-mail, votre adresse IP, les chemins de fichiers ni vos actions en jeu.<br/><br/>Vous pouvez modifier ce choix plus tard dans <b>Paramètres</b>.",
    "Would you like to send anonymous crash reports and usage data?": "Souhaitez-vous envoyer des rapports de plantage et données d’usage anonymes ?",
    "Yes, send": "Oui, envoyer",
    "You cleared the field in %1 seconds.": "Vous avez déminé le terrain en %1 secondes.",
    "You stepped on a mine.": "Vous avez marché sur une mine.",
    "You won!": "Vous avez gagné !",
    "&Statistics…": "&Statistiques…",
    "Statistics": "Statistiques",
    "Difficulty": "Difficulté",
    "Played": "Jouées",
    "Won": "Gagnées",
    "Best time": "Meilleur temps",
    "Beginner": "Débutant",
    "Intermediate": "Intermédiaire",
    "Expert": "Expert",
    "Reset all": "Tout réinitialiser",
    "Reset statistics?": "Réinitialiser les statistiques ?",
    "Permanently erase all played / won / best-time records?": "Effacer définitivement toutes les parties jouées / gagnées / meilleurs temps ?",
    "🏆 New record!": "🏆 Nouveau record !",
    "<p><small>Built with Qt %1 on %2</small></p>": "<p><small>Compilé avec Qt %1 le %2</small></p>",
    "Enable &question marks": "Activer les &points d’interrogation",
}

DE: dict[str, str] = {
    "&About": "&Über",
    "&Beginner  (9×9, 10 mines)": "&Anfänger  (9×9, 10 Minen)",
    "&Difficulty": "&Schwierigkeit",
    "&Expert  (30×16, 99 mines)": "&Experte  (30×16, 99 Minen)",
    "&Game": "&Spiel",
    "&Help": "&Hilfe",
    "&Intermediate  (16×16, 40 mines)": "&Fortgeschritten  (16×16, 40 Minen)",
    "&Language": "&Sprache",
    "&New": "&Neu",
    "&Quit": "&Beenden",
    "&Settings": "&Einstellungen",
    "<h3>QMineSweeper %1</h3><p>A Qt6-based Minesweeper game.</p><p>Left-click to reveal, right-click to flag, middle-click on a satisfied number to chord.</p><p>© Mavrikant</p>":
        "<h3>QMineSweeper %1</h3><p>Ein auf Qt6 basierendes Minesweeper-Spiel.</p><p>Linksklick zum Aufdecken, Rechtsklick zum Markieren, Mittelklick auf eine erfüllte Zahl, um die Nachbarn gleichzeitig aufzudecken.</p><p>© Mavrikant</p>",
    "About QMineSweeper": "Über QMineSweeper",
    "Auto (system)": "Automatisch (System)",
    "Boom": "Bumm!",
    "Close": "Schließen",
    "Help improve QMineSweeper": "Hilf mit, QMineSweeper zu verbessern",
    "Language changed": "Sprache geändert",
    "Language changes take effect after restart.": "Sprachänderungen wirken sich nach einem Neustart aus.",
    "Later": "Später",
    "New Game": "Neues Spiel",
    "No thanks": "Nein, danke",
    "QMineSweeper — Boom": "QMineSweeper — Bumm!",
    "QMineSweeper — Playing": "QMineSweeper — Läuft",
    "QMineSweeper — You won!": "QMineSweeper — Gewonnen!",
    "Restart QMineSweeper now?": "QMineSweeper jetzt neu starten?",
    "Restart now": "Jetzt neu starten",
    "Send anonymous &crash reports and usage data": "Anonyme &Absturzberichte und Nutzungsdaten senden",
    "We collect: app crashes, game results (win/loss, duration, difficulty), OS name, CPU architecture, Qt version, and an anonymous install ID.<br/><br/>We do <b>not</b> collect: your name, email, IP address, file paths, or any in-game actions.<br/><br/>You can change this later in <b>Settings</b>.":
        "Wir erfassen: Anwendungsabstürze, Spielergebnisse (Sieg/Niederlage, Dauer, Schwierigkeit), Name des Betriebssystems, CPU-Architektur, Qt-Version und eine anonyme Installations-ID.<br/><br/>Wir erfassen <b>nicht</b>: Ihren Namen, Ihre E-Mail, Ihre IP-Adresse, Dateipfade oder Ihre Spielaktionen.<br/><br/>Sie können dies später unter <b>Einstellungen</b> ändern.",
    "Would you like to send anonymous crash reports and usage data?": "Möchten Sie anonyme Absturzberichte und Nutzungsdaten senden?",
    "Yes, send": "Ja, senden",
    "You cleared the field in %1 seconds.": "Du hast das Feld in %1 Sekunden geräumt.",
    "You stepped on a mine.": "Du bist auf eine Mine getreten.",
    "You won!": "Gewonnen!",
    "&Statistics…": "&Statistiken…",
    "Statistics": "Statistiken",
    "Difficulty": "Schwierigkeit",
    "Played": "Gespielt",
    "Won": "Gewonnen",
    "Best time": "Bestzeit",
    "Beginner": "Anfänger",
    "Intermediate": "Fortgeschritten",
    "Expert": "Experte",
    "Reset all": "Alles zurücksetzen",
    "Reset statistics?": "Statistiken zurücksetzen?",
    "Permanently erase all played / won / best-time records?": "Alle Spiel-/Sieg-/Bestzeit-Einträge dauerhaft löschen?",
    "🏆 New record!": "🏆 Neuer Rekord!",
    "<p><small>Built with Qt %1 on %2</small></p>": "<p><small>Erstellt mit Qt %1 am %2</small></p>",
    "Enable &question marks": "&Fragezeichen aktivieren",
}

RU: dict[str, str] = {
    "&About": "&О программе",
    "&Beginner  (9×9, 10 mines)": "&Новичок  (9×9, 10 мин)",
    "&Difficulty": "С&ложность",
    "&Expert  (30×16, 99 mines)": "&Эксперт  (30×16, 99 мин)",
    "&Game": "&Игра",
    "&Help": "&Справка",
    "&Intermediate  (16×16, 40 mines)": "&Любитель  (16×16, 40 мин)",
    "&Language": "&Язык",
    "&New": "&Новая",
    "&Quit": "&Выход",
    "&Settings": "&Настройки",
    "<h3>QMineSweeper %1</h3><p>A Qt6-based Minesweeper game.</p><p>Left-click to reveal, right-click to flag, middle-click on a satisfied number to chord.</p><p>© Mavrikant</p>":
        "<h3>QMineSweeper %1</h3><p>Игра «Сапёр» на базе Qt6.</p><p>Левый клик — открыть, правый — флаг, средний клик на открытой цифре — открыть нераспознанных соседей.</p><p>© Mavrikant</p>",
    "About QMineSweeper": "О QMineSweeper",
    "Auto (system)": "Автоматически (системная)",
    "Boom": "Бум!",
    "Close": "Закрыть",
    "Help improve QMineSweeper": "Помогите улучшить QMineSweeper",
    "Language changed": "Язык изменён",
    "Language changes take effect after restart.": "Изменения языка вступят в силу после перезапуска.",
    "Later": "Позже",
    "New Game": "Новая игра",
    "No thanks": "Нет, спасибо",
    "QMineSweeper — Boom": "QMineSweeper — Бум!",
    "QMineSweeper — Playing": "QMineSweeper — Идёт игра",
    "QMineSweeper — You won!": "QMineSweeper — Вы победили!",
    "Restart QMineSweeper now?": "Перезапустить QMineSweeper сейчас?",
    "Restart now": "Перезапустить сейчас",
    "Send anonymous &crash reports and usage data": "Отправлять анонимные &отчёты о сбоях и данные использования",
    "We collect: app crashes, game results (win/loss, duration, difficulty), OS name, CPU architecture, Qt version, and an anonymous install ID.<br/><br/>We do <b>not</b> collect: your name, email, IP address, file paths, or any in-game actions.<br/><br/>You can change this later in <b>Settings</b>.":
        "Мы собираем: сбои приложения, результаты игр (победа/поражение, длительность, сложность), название ОС, архитектуру процессора, версию Qt и анонимный идентификатор установки.<br/><br/>Мы <b>не</b> собираем: ваше имя, e-mail, IP-адрес, пути к файлам или действия в игре.<br/><br/>Изменить выбор можно позже в разделе <b>Настройки</b>.",
    "Would you like to send anonymous crash reports and usage data?": "Отправлять анонимные отчёты о сбоях и данные использования?",
    "Yes, send": "Да, отправить",
    "You cleared the field in %1 seconds.": "Вы разминировали поле за %1 секунд.",
    "You stepped on a mine.": "Вы наступили на мину.",
    "You won!": "Вы победили!",
    "&Statistics…": "&Статистика…",
    "Statistics": "Статистика",
    "Difficulty": "Сложность",
    "Played": "Сыграно",
    "Won": "Побед",
    "Best time": "Лучшее время",
    "Beginner": "Новичок",
    "Intermediate": "Любитель",
    "Expert": "Эксперт",
    "Reset all": "Сбросить все",
    "Reset statistics?": "Сбросить статистику?",
    "Permanently erase all played / won / best-time records?": "Безвозвратно удалить все записи сыгранных / побед / лучших времён?",
    "🏆 New record!": "🏆 Новый рекорд!",
    "<p><small>Built with Qt %1 on %2</small></p>": "<p><small>Собрано с Qt %1 — %2</small></p>",
    "Enable &question marks": "Включить &знаки вопроса",
}

PT: dict[str, str] = {
    "&About": "&Sobre",
    "&Beginner  (9×9, 10 mines)": "&Iniciante  (9×9, 10 minas)",
    "&Difficulty": "&Dificuldade",
    "&Expert  (30×16, 99 mines)": "&Especialista  (30×16, 99 minas)",
    "&Game": "&Jogo",
    "&Help": "&Ajuda",
    "&Intermediate  (16×16, 40 mines)": "&Intermediário  (16×16, 40 minas)",
    "&Language": "&Idioma",
    "&New": "&Novo",
    "&Quit": "Sai&r",
    "&Settings": "&Configurações",
    "<h3>QMineSweeper %1</h3><p>A Qt6-based Minesweeper game.</p><p>Left-click to reveal, right-click to flag, middle-click on a satisfied number to chord.</p><p>© Mavrikant</p>":
        "<h3>QMineSweeper %1</h3><p>Um jogo Campo Minado baseado em Qt6.</p><p>Clique esquerdo para revelar, clique direito para marcar com bandeira, clique do meio sobre um número satisfeito para revelar os vizinhos.</p><p>© Mavrikant</p>",
    "About QMineSweeper": "Sobre o QMineSweeper",
    "Auto (system)": "Automático (sistema)",
    "Boom": "Bum!",
    "Close": "Fechar",
    "Help improve QMineSweeper": "Ajude a melhorar o QMineSweeper",
    "Language changed": "Idioma alterado",
    "Language changes take effect after restart.": "As alterações de idioma entram em vigor após reiniciar.",
    "Later": "Mais tarde",
    "New Game": "Novo jogo",
    "No thanks": "Não, obrigado",
    "QMineSweeper — Boom": "QMineSweeper — Bum!",
    "QMineSweeper — Playing": "QMineSweeper — Jogando",
    "QMineSweeper — You won!": "QMineSweeper — Você venceu!",
    "Restart QMineSweeper now?": "Reiniciar o QMineSweeper agora?",
    "Restart now": "Reiniciar agora",
    "Send anonymous &crash reports and usage data": "Enviar relatórios de &falhas e dados de uso anônimos",
    "We collect: app crashes, game results (win/loss, duration, difficulty), OS name, CPU architecture, Qt version, and an anonymous install ID.<br/><br/>We do <b>not</b> collect: your name, email, IP address, file paths, or any in-game actions.<br/><br/>You can change this later in <b>Settings</b>.":
        "Coletamos: falhas do aplicativo, resultados de partidas (vitória/derrota, duração, dificuldade), nome do sistema operacional, arquitetura de CPU, versão do Qt e um ID de instalação anônimo.<br/><br/><b>Não</b> coletamos: seu nome, e-mail, endereço IP, caminhos de arquivos nem ações dentro do jogo.<br/><br/>Você pode alterar isso depois em <b>Configurações</b>.",
    "Would you like to send anonymous crash reports and usage data?": "Você gostaria de enviar relatórios de falhas e dados de uso anônimos?",
    "Yes, send": "Sim, enviar",
    "You cleared the field in %1 seconds.": "Você limpou o campo em %1 segundos.",
    "You stepped on a mine.": "Você pisou em uma mina.",
    "You won!": "Você venceu!",
    "&Statistics…": "&Estatísticas…",
    "Statistics": "Estatísticas",
    "Difficulty": "Dificuldade",
    "Played": "Jogadas",
    "Won": "Vitórias",
    "Best time": "Melhor tempo",
    "Beginner": "Iniciante",
    "Intermediate": "Intermediário",
    "Expert": "Especialista",
    "Reset all": "Redefinir tudo",
    "Reset statistics?": "Redefinir estatísticas?",
    "Permanently erase all played / won / best-time records?": "Apagar permanentemente todos os registros de partidas / vitórias / melhores tempos?",
    "🏆 New record!": "🏆 Novo recorde!",
    "<p><small>Built with Qt %1 on %2</small></p>": "<p><small>Compilado com Qt %1 em %2</small></p>",
    "Enable &question marks": "Ativar pontos de &interrogação",
}

ZH: dict[str, str] = {
    "&About": "关于(&A)",
    "&Beginner  (9×9, 10 mines)": "初级(&B)  (9×9, 10 颗雷)",
    "&Difficulty": "难度(&D)",
    "&Expert  (30×16, 99 mines)": "高级(&E)  (30×16, 99 颗雷)",
    "&Game": "游戏(&G)",
    "&Help": "帮助(&H)",
    "&Intermediate  (16×16, 40 mines)": "中级(&I)  (16×16, 40 颗雷)",
    "&Language": "语言(&L)",
    "&New": "新建(&N)",
    "&Quit": "退出(&Q)",
    "&Settings": "设置(&S)",
    "<h3>QMineSweeper %1</h3><p>A Qt6-based Minesweeper game.</p><p>Left-click to reveal, right-click to flag, middle-click on a satisfied number to chord.</p><p>© Mavrikant</p>":
        "<h3>QMineSweeper %1</h3><p>基于 Qt6 的扫雷游戏。</p><p>左键揭示，右键标记旗帜，在满足数字的格子上按中键可同时揭开周围格子。</p><p>© Mavrikant</p>",
    "About QMineSweeper": "关于 QMineSweeper",
    "Auto (system)": "自动（系统）",
    "Boom": "轰！",
    "Close": "关闭",
    "Help improve QMineSweeper": "帮助改进 QMineSweeper",
    "Language changed": "语言已更改",
    "Language changes take effect after restart.": "语言更改将在重启后生效。",
    "Later": "稍后",
    "New Game": "新游戏",
    "No thanks": "不，谢谢",
    "QMineSweeper — Boom": "QMineSweeper — 轰！",
    "QMineSweeper — Playing": "QMineSweeper — 进行中",
    "QMineSweeper — You won!": "QMineSweeper — 你赢了！",
    "Restart QMineSweeper now?": "现在重启 QMineSweeper？",
    "Restart now": "立即重启",
    "Send anonymous &crash reports and usage data": "发送匿名崩溃报告和使用数据(&C)",
    "We collect: app crashes, game results (win/loss, duration, difficulty), OS name, CPU architecture, Qt version, and an anonymous install ID.<br/><br/>We do <b>not</b> collect: your name, email, IP address, file paths, or any in-game actions.<br/><br/>You can change this later in <b>Settings</b>.":
        "我们收集：应用崩溃、游戏结果（胜负、时长、难度）、操作系统名称、CPU 架构、Qt 版本以及一个匿名安装 ID。<br/><br/>我们<b>不</b>收集：您的姓名、电子邮件、IP 地址、文件路径或任何游戏内操作。<br/><br/>您可稍后在<b>设置</b>中更改。",
    "Would you like to send anonymous crash reports and usage data?": "您是否愿意发送匿名崩溃报告和使用数据？",
    "Yes, send": "好的，发送",
    "You cleared the field in %1 seconds.": "您用时 %1 秒清扫完成。",
    "You stepped on a mine.": "您踩到了雷。",
    "You won!": "你赢了！",
    "&Statistics…": "统计(&S)…",
    "Statistics": "统计",
    "Difficulty": "难度",
    "Played": "已玩",
    "Won": "获胜",
    "Best time": "最佳时间",
    "Beginner": "初级",
    "Intermediate": "中级",
    "Expert": "高级",
    "Reset all": "全部重置",
    "Reset statistics?": "重置统计数据？",
    "Permanently erase all played / won / best-time records?": "是否永久删除所有已玩、获胜与最佳时间记录？",
    "🏆 New record!": "🏆 新纪录！",
    "<p><small>Built with Qt %1 on %2</small></p>": "<p><small>使用 Qt %1 构建于 %2</small></p>",
    "Enable &question marks": "启用问号标记(&Q)",
}

HI: dict[str, str] = {
    "&About": "&परिचय",
    "&Beginner  (9×9, 10 mines)": "&शुरुआती  (9×9, 10 माइन)",
    "&Difficulty": "&कठिनाई",
    "&Expert  (30×16, 99 mines)": "&विशेषज्ञ  (30×16, 99 माइन)",
    "&Game": "&खेल",
    "&Help": "&मदद",
    "&Intermediate  (16×16, 40 mines)": "&मध्यम  (16×16, 40 माइन)",
    "&Language": "&भाषा",
    "&New": "&नया",
    "&Quit": "&बाहर निकलें",
    "&Settings": "&सेटिंग्स",
    "<h3>QMineSweeper %1</h3><p>A Qt6-based Minesweeper game.</p><p>Left-click to reveal, right-click to flag, middle-click on a satisfied number to chord.</p><p>© Mavrikant</p>":
        "<h3>QMineSweeper %1</h3><p>Qt6 पर आधारित माइनस्वीपर खेल।</p><p>बाएँ क्लिक से खोलें, दाएँ क्लिक से झंडा लगाएँ, संख्या पर मध्य क्लिक से आसपास के पड़ोसी खोलें।</p><p>© Mavrikant</p>",
    "About QMineSweeper": "QMineSweeper के बारे में",
    "Auto (system)": "स्वतः (सिस्टम)",
    "Boom": "धमाका!",
    "Close": "बंद करें",
    "Help improve QMineSweeper": "QMineSweeper को बेहतर बनाने में मदद करें",
    "Language changed": "भाषा बदली गई",
    "Language changes take effect after restart.": "भाषा परिवर्तन पुनः आरंभ के बाद लागू होंगे।",
    "Later": "बाद में",
    "New Game": "नया खेल",
    "No thanks": "नहीं, धन्यवाद",
    "QMineSweeper — Boom": "QMineSweeper — धमाका!",
    "QMineSweeper — Playing": "QMineSweeper — खेल जारी है",
    "QMineSweeper — You won!": "QMineSweeper — आप जीत गए!",
    "Restart QMineSweeper now?": "क्या अभी QMineSweeper को पुनः आरंभ करें?",
    "Restart now": "अभी पुनः आरंभ करें",
    "Send anonymous &crash reports and usage data": "अनाम &क्रैश रिपोर्ट और उपयोग डेटा भेजें",
    "We collect: app crashes, game results (win/loss, duration, difficulty), OS name, CPU architecture, Qt version, and an anonymous install ID.<br/><br/>We do <b>not</b> collect: your name, email, IP address, file paths, or any in-game actions.<br/><br/>You can change this later in <b>Settings</b>.":
        "हम एकत्र करते हैं: ऐप क्रैश, खेल परिणाम (जीत/हार, अवधि, कठिनाई), OS नाम, CPU आर्किटेक्चर, Qt संस्करण और एक अनाम स्थापना ID।<br/><br/>हम <b>संग्रह नहीं करते</b>: आपका नाम, ईमेल, IP पता, फ़ाइल पथ या खेल की कोई क्रिया।<br/><br/>आप इसे बाद में <b>सेटिंग्स</b> में बदल सकते हैं।",
    "Would you like to send anonymous crash reports and usage data?": "क्या आप अनाम क्रैश रिपोर्ट और उपयोग डेटा भेजना चाहेंगे?",
    "Yes, send": "हाँ, भेजें",
    "You cleared the field in %1 seconds.": "आपने %1 सेकंड में मैदान साफ किया।",
    "You stepped on a mine.": "आप एक माइन पर कदम रख गए।",
    "You won!": "आप जीत गए!",
    "&Statistics…": "&आँकड़े…",
    "Statistics": "आँकड़े",
    "Difficulty": "कठिनाई",
    "Played": "खेले",
    "Won": "जीते",
    "Best time": "सर्वश्रेष्ठ समय",
    "Beginner": "शुरुआती",
    "Intermediate": "मध्यम",
    "Expert": "विशेषज्ञ",
    "Reset all": "सभी रीसेट करें",
    "Reset statistics?": "आँकड़े रीसेट करें?",
    "Permanently erase all played / won / best-time records?": "क्या सभी खेल / जीत / सर्वश्रेष्ठ समय के रिकॉर्ड स्थायी रूप से मिटा दिए जाएँ?",
    "🏆 New record!": "🏆 नया रिकॉर्ड!",
    "<p><small>Built with Qt %1 on %2</small></p>": "<p><small>Qt %1 के साथ %2 को निर्मित</small></p>",
    "Enable &question marks": "&प्रश्न चिह्न सक्षम करें",
}

AR: dict[str, str] = {
    "&About": "&حول",
    "&Beginner  (9×9, 10 mines)": "&مبتدئ  (9×9، 10 ألغام)",
    "&Difficulty": "ال&صعوبة",
    "&Expert  (30×16, 99 mines)": "&خبير  (30×16، 99 لغماً)",
    "&Game": "ال&لعبة",
    "&Help": "ال&مساعدة",
    "&Intermediate  (16×16, 40 mines)": "&متوسط  (16×16، 40 لغماً)",
    "&Language": "ال&لغة",
    "&New": "&جديد",
    "&Quit": "&خروج",
    "&Settings": "الإ&عدادات",
    "<h3>QMineSweeper %1</h3><p>A Qt6-based Minesweeper game.</p><p>Left-click to reveal, right-click to flag, middle-click on a satisfied number to chord.</p><p>© Mavrikant</p>":
        "<h3>QMineSweeper %1</h3><p>لعبة كانسة الألغام مبنية على Qt6.</p><p>انقر يسارياً للكشف، يميناً لوضع علم، وبالزر الأوسط على رقم مشبع لفتح الجيران.</p><p>© Mavrikant</p>",
    "About QMineSweeper": "حول QMineSweeper",
    "Auto (system)": "تلقائي (النظام)",
    "Boom": "بوم!",
    "Close": "إغلاق",
    "Help improve QMineSweeper": "ساعد في تحسين QMineSweeper",
    "Language changed": "تم تغيير اللغة",
    "Language changes take effect after restart.": "تُطبَّق تغييرات اللغة بعد إعادة التشغيل.",
    "Later": "لاحقاً",
    "New Game": "لعبة جديدة",
    "No thanks": "لا، شكراً",
    "QMineSweeper — Boom": "QMineSweeper — بوم!",
    "QMineSweeper — Playing": "QMineSweeper — قيد اللعب",
    "QMineSweeper — You won!": "QMineSweeper — لقد فزت!",
    "Restart QMineSweeper now?": "هل تعيد تشغيل QMineSweeper الآن؟",
    "Restart now": "إعادة التشغيل الآن",
    "Send anonymous &crash reports and usage data": "إرسال تقارير الأعطال وبيانات الاستخدام المجهولة",
    "We collect: app crashes, game results (win/loss, duration, difficulty), OS name, CPU architecture, Qt version, and an anonymous install ID.<br/><br/>We do <b>not</b> collect: your name, email, IP address, file paths, or any in-game actions.<br/><br/>You can change this later in <b>Settings</b>.":
        "نجمع: أعطال التطبيق، نتائج المباريات (فوز/خسارة، المدة، الصعوبة)، اسم نظام التشغيل، معمارية المعالج، إصدار Qt، ومعرّف تثبيت مجهول.<br/><br/>لا نجمع: اسمك، بريدك الإلكتروني، عنوان IP، مسارات الملفات، أو أي إجراءات داخل اللعبة.<br/><br/>يمكنك تغيير هذا لاحقاً من <b>الإعدادات</b>.",
    "Would you like to send anonymous crash reports and usage data?": "هل تريد إرسال تقارير الأعطال وبيانات الاستخدام المجهولة؟",
    "Yes, send": "نعم، أرسل",
    "You cleared the field in %1 seconds.": "نظّفت الحقل في %1 ثانية.",
    "You stepped on a mine.": "لقد دست على لغم.",
    "You won!": "لقد فزت!",
    "&Statistics…": "ال&إحصائيات…",
    "Statistics": "الإحصائيات",
    "Difficulty": "الصعوبة",
    "Played": "اللعبات",
    "Won": "الانتصارات",
    "Best time": "أفضل وقت",
    "Beginner": "مبتدئ",
    "Intermediate": "متوسط",
    "Expert": "خبير",
    "Reset all": "تصفير الكل",
    "Reset statistics?": "تصفير الإحصائيات؟",
    "Permanently erase all played / won / best-time records?": "حذف جميع سجلات المباريات / الانتصارات / أفضل الأوقات نهائياً؟",
    "🏆 New record!": "🏆 رقم قياسي جديد!",
    "<p><small>Built with Qt %1 on %2</small></p>": "<p><small>بُني باستخدام Qt %1 في %2</small></p>",
    "Enable &question marks": "تفعيل علامات الاست&فهام",
}

LOCALES: dict[str, dict[str, str]] = {
    "en": EN,
    "tr_TR": TR,
    "es_ES": ES,
    "fr_FR": FR,
    "de_DE": DE,
    "ru_RU": RU,
    "pt_BR": PT,
    "zh_CN": ZH,
    "hi_IN": HI,
    "ar_SA": AR,
}


def unescape_source(s: str) -> str:
    # .ts XML uses the same entity set we need to reverse.
    return (
        s.replace("&amp;", "&")
        .replace("&lt;", "<")
        .replace("&gt;", ">")
        .replace("&quot;", '"')
        .replace("&apos;", "'")
    )


MESSAGE_RE = re.compile(
    r"(<message>\s*(?:<location\s[^>]*/>\s*)+<source>)"
    r"(.*?)"
    r"(</source>\s*<translation)"
    r'(\s+type="unfinished")?'
    r"(\s*>)"
    r"(.*?)"
    r"(</translation>\s*</message>)",
    re.DOTALL,
)


def apply(locale: str, mapping: dict[str, str]) -> int:
    path = ROOT / f"QMineSweeper_{locale}.ts"
    text = path.read_text(encoding="utf-8")
    updates = 0

    def repl(m: re.Match[str]) -> str:
        nonlocal updates
        head, source_escaped, tail_open, _unfinished_attr, gt, _body, tail_close = m.groups()
        source = unescape_source(source_escaped)
        if source in mapping:
            translated = xml_escape(mapping[source])
            updates += 1
            return f"{head}{source_escaped}{tail_open}{gt}{translated}{tail_close}"
        if source in NO_TRANSLATE:
            # Empty translation, marked finished → Qt falls back to source.
            return f"{head}{source_escaped}{tail_open}{gt}{tail_close}"
        # Leave unfinished if we have no translation.
        return m.group(0)

    new_text = MESSAGE_RE.sub(repl, text)
    if new_text != text:
        path.write_text(new_text, encoding="utf-8")
    print(f"{locale}: {updates} translations applied")
    return updates


def main() -> None:
    for locale, mapping in LOCALES.items():
        apply(locale, mapping)


if __name__ == "__main__":
    main()
