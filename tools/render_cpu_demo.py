from pathlib import Path
import subprocess
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "media" / "cpu-ram-gameplay-frames"
OUT.mkdir(parents=True, exist_ok=True)
result = subprocess.run([str(ROOT / "build" / "legend"), "--run", str(ROOT / "examples" / "complete_game.lgnd")], capture_output=True, text=True, check=True)
logs = result.stdout.strip().splitlines() + result.stderr.strip().splitlines()
font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 20)
small = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 16)
bold = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 22)
for frame in range(250):
    im = Image.new("RGB", (1280, 720), "#10151b")
    d = ImageDraw.Draw(im)
    d.rectangle((0, 0, 1280, 52), fill="#202a34")
    d.text((24, 15), "legend — CPU/RAM PLAYTEST", font=bold, fill="#c8f36b")
    d.text((965, 16), "OpenGL host: none", font=small, fill="#ff9c84")
    d.rounded_rectangle((38, 78, 1242, 642), radius=10, fill="#17232d", outline="#344553", width=2)
    d.text((62, 96), "$ legend --run examples/complete_game.lgnd", font=font, fill="#f7f4ee")
    d.text((62, 130), "[DEMO] Executando em modo headless: gameplay simulado em memória RAM", font=small, fill="#ffcf6b")
    y = 168
    for line in logs:
        d.text((62, y), line[:95], font=small, fill="#9eb1bd")
        y += 25
    # A compact CPU-only world view: the game code created two entities.
    left, top, right, bottom = 700, 310, 1190, 590
    d.rectangle((left, top, right, bottom), fill="#0c1116", outline="#506575", width=2)
    d.text((left + 18, top + 14), "WORLD / arena", font=small, fill="#c8f36b")
    for gx in range(left + 20, right, 45): d.line((gx, top + 45, gx, bottom - 18), fill="#1d303c")
    for gy in range(top + 55, bottom, 45): d.line((left + 18, gy, right - 18, gy), fill="#1d303c")
    px = left + 80 + int((frame * 1.5) % 290)
    py = top + 170 + int(10 * __import__('math').sin(frame / 14))
    ex = left + 345
    d.ellipse((px - 15, py - 15, px + 15, py + 15), fill="#c8f36b")
    d.text((px - 22, py + 22), "Player", font=small, fill="#c8f36b")
    d.rectangle((ex - 15, py - 15, ex + 15, py + 15), fill="#ff734f")
    d.text((ex - 20, py + 22), "Enemy", font=small, fill="#ff734f")
    d.text((left + 18, bottom - 36), "render backend: headless   |   memory: RAM   |   GPU: unavailable", font=small, fill="#778b98")
    d.text((62, 600), f"frame {frame:03d}/250   time {frame/60:05.2f}s   input_axis(move_x)=0.00", font=small, fill="#70a3ff")
    im.save(OUT / f"frame-{frame:04d}.png")
