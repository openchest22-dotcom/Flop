#!/bin/bash

# Dosyanın mevcut dizinde olup olmadığını kontrol et
if [ -f "flop" ]; then
    echo "flop dosyası bulunuyor, taşıma işlemi başlıyor..."
    
    # Dosyayı /usr/local/bin altına kopyala
    sudo cp flop /usr/local/bin/
    
    # Çalıştırılabilir olması için izinleri ayarla
    sudo chmod +x /usr/local/bin/flop
    
    echo "Başarılı! Flop kullanılabilir!."
else
    echo "Hata: Mevcut dizinde 'flop' dosyası bulunamadı!"
    exit 1
fi