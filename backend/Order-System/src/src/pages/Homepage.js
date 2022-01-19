import React, { useState } from "react";
import Picture from "../components/Picture";
import ConfirmForm from "./ConfirmForm";
import axios from "axios";

import { Button } from "react-bootstrap";

const Homepage = () => {
  const [show, setShow] = useState(false);
  const [total, setTotal] = useState(0);
  const [Product, setProduct] = useState([0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);

  let data = [
    { index: "01", name: "牛肉麵", price: 120 },
    { index: "02", name: "小籠包", price: 90 },
    { index: "03", name: "滷肉飯", price: 30 },
    { index: "04", name: "蚵仔麵線", price: 60 },
    { index: "05", name: "蚵仔煎", price: 55 },
    { index: "06", name: "臭豆腐", price: 50 },
    { index: "07", name: "雞排", price: 65 },
    { index: "08", name: "珍珠奶茶", price: 45 },
    { index: "09", name: "刨冰", price: 50 },
    { index: "10", name: "鳳梨酥", price: 80 },
  ];
  const addHandler = (index, amount) => {
    setProduct((prev) => {
      let newProduct = [...prev];
      newProduct[index] += amount;
      return newProduct;
    });
  };

  const totalPriceHandler = (price) => {
    setTotal((previousTotal) => previousTotal + price);
  };

  const resetHandler = () => {
    setTotal(0);
    setProduct([0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
    setShow(false);
  };

  const confirmHandler = () => {
    setShow(false);
  };
  const submitHandler = () => {
    console.log(Product);
    setShow(true);
  };
  const closeHandler = () => {
    setShow(false);
  };
  // **   send   request    **//
  const sendHandler = () => {
    setShow(false);
    axios.post("https://indoor-foodie.herokuapp.com/Menu", {
      product: Product,
      total: total,
    });
  };
  // **    //   **     //   **//

  return (
    <div style={{ minHeight: "100vh" }}>
      {show && (
        <ConfirmForm
          onConfirm={confirmHandler}
          onCloseForm={closeHandler}
          onSend={sendHandler}
          show={show}
          product={Product}
          price={total}
        />
      )}
      <div className="pictures">
        {data &&
          data.map((d, index) => {
            return (
              <Picture
                data={d.index}
                name={d.name}
                price={d.price}
                number={Product[index]}
                index={index}
                addHandler={addHandler}
                totalPriceHandler={totalPriceHandler}
              />
            );
          })}
      </div>
      <div className=" more">
        <Button variant="outline-danger" className="btn btn-sm" onClick={resetHandler}>
          Reset
        </Button>
        <Button variant="outline-success" className="btn btn-sm" onClick={submitHandler}>
          Submit
        </Button>
      </div>
    </div>
  );
};

export default Homepage;
