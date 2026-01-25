#pragma once
#include <map>
#include <string>
#include <memory>
#include <stdexcept>

// Erişim için benzersiz anahtarlar
enum class ServiceID {
    MainFrame,
    TreeListDialog,
    CustomerListView,
    HomeDialog,
    DBManager,
    // Eklemek istediğiniz diğer modüller...
};

class AppContext
{
public:
    // Singleton Erişimi
    static AppContext& Get() {
        static AppContext instance;
        return instance;
    }

    // Nesne Kayıt Etme (Örn: OnCreate içinde)
    template <typename T>
    void Register(ServiceID id, T* instance) {
        m_registry[id] = static_cast<void*>(instance);
    }

    // Kayıt Silme (Örn: OnDestroy içinde)
    void Unregister(ServiceID id) {
        m_registry.erase(id);
    }

    // Nesneye Erişim (Her yerden çağrılabilir)
    template <typename T>
    T* Resolve(ServiceID id) {
        auto it = m_registry.find(id);
        if (it != m_registry.end()) {
            return static_cast<T*>(it->second);
        }
        return nullptr; // Nesne henüz oluşmamış veya silinmiş
    }

private:
    AppContext() = default;
    std::map<ServiceID, void*> m_registry;
};

// ==========================================================
// 🚀 KISAYOL (SYNTACTIC SUGAR)
// Bu namespace sayesinde AppContext::Get().Resolve... yazmak zorunda kalmazsınız.
// ==========================================================

// Forward declarations (Headerları include etmemek için)
class CMainFrame;
class CTreeListVDialog;
class CAdvancedListView;
class CDoc;

namespace App
{
    // Kullanım kolaylığı için statik yardımcılar
    inline CMainFrame* MainFrame() { 
        return AppContext::Get().Resolve<CMainFrame>(ServiceID::MainFrame); 
    }

    inline CTreeListVDialog* TreeList() { 
        return AppContext::Get().Resolve<CTreeListVDialog>(ServiceID::TreeListDialog); 
    }
    
    inline CAdvancedListView* CustomerList() {
        return AppContext::Get().Resolve<CAdvancedListView>(ServiceID::CustomerListView);
    }
    
    // İhtiyaç oldukça buraya ekleyin...
}