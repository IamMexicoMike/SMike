#include <iostream>
#include "asio.hpp"
#include <opencv2/opencv.hpp>
#include "extraccion_codificacion.h"

using namespace std;
using namespace asio;
using namespace cv;

vector<char> cuadro_global(1024*1024);

/**consiste de un socket cliente, un puerto serial, un nombre de servicio a proveer*/
class fwd
{
public:
  fwd(asio::io_service& io_service, std::string ip, std::string puerto_tcp) :
    iosvc_(io_service),
    socket_(io_service),
    temporizador_(io_service)
  {
    asio::ip::tcp::resolver resolvedor(io_service);
    asio::ip::tcp::resolver::query query(ip, puerto_tcp); //puedes meter dns aqui
    asio::ip::tcp::resolver::iterator iter = resolvedor.resolve(query);
    endpoint_ = iter->endpoint();
    tx_buf_socket_.clear();
  }
  void iniciar()
  {
    iniciar_temporizador();
    conectar_socket();
  }

  void conectar_socket();
  void leer_socket();
  void escribir_socket(std::string str);


private:
  void iniciar_temporizador();

  asio::io_service& iosvc_;

  std::array<char,8192> rx_buf_socket_;
  std::string tx_buf_socket_;
  asio::ip::tcp::socket socket_;
  asio::ip::tcp::endpoint endpoint_;

  asio::steady_timer temporizador_;
  bool cerrar_{false};
};

/**                 socket                       */
/**Funci�n miembro que verifica cada 25ms si hay algo que debamos enviar al servidor*/
void fwd::iniciar_temporizador()
{
  asio::error_code ec;
  temporizador_.expires_from_now(std::chrono::milliseconds(2000), ec);
  temporizador_.async_wait([this](std::error_code ec)
  {
    if(!ec)
    {
      Mat m(ScreenCap_Ipl());
      escribir_socket(codificar(m));
    }

    else
      std::cout << "Error temporizador: " << ec.value() <<  ": " <<  ec.message() << std::endl;

    if(!cerrar_)
      iniciar_temporizador();
  });
}


void fwd::conectar_socket()
{
  socket_.async_connect(endpoint_, [this](error_code ec)
  {
    if(!ec)
    {
      cout << "conectado a " << this->socket_.remote_endpoint().address().to_string()
          << ":" << this->socket_.remote_endpoint().port() << '\n';
      string login="mike;ftw;";
      cout << "LOGIN: " << login;
      escribir_socket(login);
      Sleep(200);
      string peticion="ofrecer desk";
      escribir_socket(peticion);

      leer_socket();
    }

    else
    {
      cout << "Error conectando: " << ec.value() <<  ": " <<  ec.message() << '\n';
      if(ec.value() == 10056) /*Error 10056: Se solicit� conexi�n en socket ya conectado*/
      {
        std::error_code ec_cerrar;
        socket_.close(ec_cerrar);
        if(!ec_cerrar)
          conectar_socket();
        else
          std::cout << "Error cerrando y conectando: " << ec_cerrar.value() <<  ": " <<  ec_cerrar.message() << std::endl;
      }
        //...
    }
  });
} //conectar

void fwd::leer_socket()
{
  socket_.async_read_some(asio::buffer(rx_buf_socket_),
    [this](std::error_code ec, std::size_t bytes_leidos)
  {
    if(!ec)
    {
      cout.write(rx_buf_socket_.data(), bytes_leidos);

      memset(rx_buf_socket_.data(), '\0', rx_buf_socket_.size() );
      leer_socket();
    }
    else
    {
      cout << "Error leyendo socket->" << ec.value() <<  ": " << ec.message() << std::endl;
      cerrar_=true;
    }
  });
}

void fwd::escribir_socket(std::string str)
{
  tx_buf_socket_ = str;
  //cout << str;
  asio::async_write(socket_, asio::buffer(tx_buf_socket_.data(), tx_buf_socket_.size()),
    [this](std::error_code ec, std::size_t len)
  {
    if (!ec)
    {
      cout << "Se escribieron " << len << "bytes" << '\n';
      /*procesar escritura exitosa*/
    }
  });
}


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

class receptor
{
public:
  receptor(asio::io_service& io_service, std::string ip, std::string puerto_tcp) :
    iosvc_(io_service),
    socket_(io_service),
    temporizador_(io_service)
  {
    asio::ip::tcp::resolver resolvedor(io_service);
    asio::ip::tcp::resolver::query query(ip, puerto_tcp); //puedes meter dns aqui
    asio::ip::tcp::resolver::iterator iter = resolvedor.resolve(query);
    endpoint_ = iter->endpoint();
    cuadro_.reserve(1024*1024);
  }
  void iniciar()
  {
    conectar_socket();
  }

  void conectar_socket();
  void leer_socket();
  void escribir_socket(std::string str);


private:
  void iniciar_temporizador();

  asio::io_service& iosvc_;

  std::array<char,8192> rx_buf_socket_;
  std::string tx_buf_socket_;
  asio::ip::tcp::socket socket_;
  asio::ip::tcp::endpoint endpoint_;

  asio::steady_timer temporizador_;

  std::vector<char> cuadro_;
};


void receptor::conectar_socket()
{
  socket_.async_connect(endpoint_, [this](error_code ec)
  {
    if(!ec)
    {
      cout << "conectado a " << this->socket_.remote_endpoint().address().to_string()
          << ":" << this->socket_.remote_endpoint().port() << '\n';
      escribir_socket("nube;nube;");
      Sleep(200);
      escribir_socket("suscribir desk");

      leer_socket();
    }

    else
    {
      cout << "Error conectando: " << ec.value() <<  ": " <<  ec.message() << '\n';
      if(ec.value() == 10056) /*Error 10056: Se solicit� conexi�n en socket ya conectado*/
      {
        std::error_code ec_cerrar;
        socket_.close(ec_cerrar);
        if(!ec_cerrar)
          conectar_socket();
        else
          std::cout << "Error cerrando y conectando: " << ec_cerrar.value() <<  ": " <<  ec_cerrar.message() << std::endl;
      }
        //...
    }
  });
} //conectar

void receptor::leer_socket()
{
  socket_.async_read_some(asio::buffer(rx_buf_socket_),
    [this](std::error_code ec, std::size_t bytes_leidos)
  {
    if(!ec)
    {
      cout << "cuadro_.size()=" << cuadro_.size() << "+ bytes_leidos=" << bytes_leidos ;
      cuadro_.insert(cuadro_.end(), rx_buf_socket_.begin(), rx_buf_socket_.begin() + bytes_leidos);
      cout << " ==> cuadro.size()=" << cuadro_.size() << '\n';
      //cout.write(rx_buf_socket_.data(), bytes_leidos);
      cout << "ultimos 2 bytes: " << (int)rx_buf_socket_[bytes_leidos-2] <<" "<<(int)rx_buf_socket_[bytes_leidos-1] <<"\n";
      if((int)rx_buf_socket_[bytes_leidos-2]==-1 && (int)rx_buf_socket_[bytes_leidos-1] == -39 )
      {
        cout << "fin de frame. cuadro_=" <<cuadro_.size() << '\n' ;//<< "\tframe w,h="<< frame.cols << "," <<frame.rows <<"\n";
        cuadro_global=cuadro_;
        cuadro_.clear();
      }


      memset(rx_buf_socket_.data(), '\0', rx_buf_socket_.size() );
      leer_socket();
    }
    else
    {
      cout << "Error leyendo socket->" << ec.value() <<  ": " << ec.message() << std::endl;
    }
  });
}

void receptor::escribir_socket(std::string str)
{
  tx_buf_socket_ = str;
  asio::async_write(socket_, asio::buffer(tx_buf_socket_.data(), tx_buf_socket_.size()),
    [this](std::error_code ec, std::size_t len)
  {
    if (!ec)
    {
      //cout << "Se escribio " << tx_buf_ << " con " << len << " bytes" << endl;
      /*procesar escritura exitosa*/
    }
  });
}

void renderizar()
{
  cv::redirectError(solucionador_de_pedos);
  namedWindow("cuadro"); //una cicatriz en la cara de Par�s
  while(waitKey(300) != 'q')
  {
    if(cuadro_global.size() > 0)
    {
      try
      {
        Mat frame = cv::imdecode(cuadro_global, CV_LOAD_IMAGE_GRAYSCALE);
        imshow("cuadro",frame);
      }
      catch(std::exception const& e)
      {
        cout << ".";// << e.what();
      }
    }

  }
}

int main(int argc, char* argv[])
{
  if(argc != 2)
  {
    cout << argc <<" Por favor ingresa como argumento del programa \"cliente\" o \"servidor\" dependiendo de que quieras\n";
    exit(0);
  }

  try
  {
    io_service servicio;

    if(strcmp( argv[1], "servidor") == 0)
    {
      fwd escaner_escritorio(servicio, "201.139.98.214", "3214");
      escaner_escritorio.iniciar();
      servicio.run();
    }

    else if(strcmp( argv[1], "cliente") == 0)
    {
      thread t(renderizar);
      receptor receptor_escritorio(servicio, "201.139.98.214", "3214");
      receptor_escritorio.iniciar();
      servicio.run();
      t.join();
    }

    else
    {
      cout << "\nPor favor ingresa una opcion correcta\n";
    }

  }
  catch(std::exception const& e)
  {
    cout << e.what() << '\n';
  }

  return 0;
}
